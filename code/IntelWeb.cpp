#define _CRT_SECURE_NO_DEPRECATE
#include "IntelWeb.h"
#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

IntelWeb::IntelWeb(){}

IntelWeb::~IntelWeb(){
	datafrom.close();
	datato.close();
}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems) {
	//close any DiskMultiMaps this IntelWeb object might have created or opened before creating any new ones
	close();

	//create two DiskMultiMaps
	if (datafrom.createNew(filePrefix + "-datafrom.dat", 2 * maxDataItems) && datato.createNew(filePrefix + "-datato.dat", 2 * maxDataItems))
		return true;  //if successfully created, return true
	else {    //if not successfully created, return false
		close();
		return false;
	}
}

bool IntelWeb::openExisting(const std::string& filePrefix) {
	//close any DiskMultiMaps this IntelWeb object might have created or opened before creating any new ones
	close();

	//open existing two DiskMultiMaps
	if (datafrom.openExisting(filePrefix + "-datafrom.dat") && datato.openExisting(filePrefix + "-datato.dat"))
		return true;  //if successfully created, return true
	else {    //if not successfully created, return false
		close();
		return false;
	}
}

void IntelWeb::close() {
	//close any DiskMultiMaps this IntelWeb object might have created or opened before creating any new ones
	datafrom.close();
	datato.close();
}

bool IntelWeb::ingest(const std::string& telemetryFile) {
	// Open the file for input
	ifstream inf(telemetryFile);
	// Test for failure to open
	if (!inf)
	{
		cout << "Cannot open file!" << endl;
		return false;
	}
	string line;
	while (getline(inf, line)) {
		istringstream iss(line); //create an input stringstream
		string from, to, context;
		// if we can't extract three strings
		if (!(iss >> context >> from >> to))
		{
			cout << "Ignoring badly-formatted input line: " << line << endl;
			continue;
		}
		datafrom.insert(from, to, context);
		datato.insert(to, from, context);
	}
	return true;
}

void IntelWeb::print() {
	//datafrom.printAll();
	cout << "==================" << endl;
	datato.printAll();
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
	unsigned int minPrevalenceToBeGood,
	std::vector<std::string>& badEntitiesFound,
	std::vector<InteractionTuple>& interactions
	) {
	//clear both vectors
	badEntitiesFound.clear();
	interactions.clear();
	queue<string> bfs;     //queue used for BFS
	unordered_set<string> visitedEntity;  //record which entity has been visited in BFS
	unordered_set<string> visitedInteraction;  //record which interacion has been visited in BFS

	for (unsigned int i = 0; i < indicators.size(); i++) { //BFS for each indicator
		string indicator = indicators[i];
		//search for the indicator
		DiskMultiMap::Iterator it1 = datafrom.search(indicator);
		DiskMultiMap::Iterator it2 = datato.search(indicator);
		//if the indication cannot be found, move to next indicator
		if ((!it1.isValid() && !it2.isValid()))
			continue;
		//indicator not visited and can be found in the hash maps
		if (visitedEntity.find(indicator) == visitedEntity.end()) {
			bfs.push(indicator);  //enqueue the indicator
			visitedEntity.insert(indicator);  //v has been visited
		}
		//BFS from this indicator
		while (!bfs.empty()) {
			//dequeue
			string bad = bfs.front();
			//cout << "enqueue item: " << bad << " prevalence: " << prevalance(bad) << endl;
			bfs.pop();
			//if the key has high prevanlence, continue to next element in the queue
			if (prevalance(bad, minPrevalenceToBeGood)) {
				continue;
			}
			//the key has low prevanlence and it malicious
			badEntitiesFound.push_back(bad);  //insert the element into badEntity (sorted for output)
			//find the location of bad
			DiskMultiMap::Iterator it1 = datafrom.search(bad);
			DiskMultiMap::Iterator it2 = datato.search(bad);
			//if it is a key in datafrom
			if (it1.isValid()) {
				do {
					//if current interation is not visited before
					string con = (*it1).key + (*it1).value + (*it1).context;
					if (visitedInteraction.find(con) == visitedInteraction.end()) {
						visitedInteraction.insert(con);   //record as visited
						InteractionTuple interT((*it1).key, (*it1).value, (*it1).context);
						interactions.push_back(interT);
					}
					string v = (*it1).value; //obtain the value
					//if v is not visited before
					if (visitedEntity.find(v) == visitedEntity.end()) {
						bfs.push(v); //enqueue the string
						visitedEntity.insert(v);  //v has been visited
					}
					++it1; //move to next key
				} while (it1.isValid());
			}
			//if it is a key in datato
			if (it2.isValid()) {
				do {
					string con = (*it2).value + (*it2).key + (*it2).context;
					if (visitedInteraction.find(con) == visitedInteraction.end()) {
						visitedInteraction.insert(con);   //record as visited
						InteractionTuple interT((*it2).value, (*it2).key, (*it2).context);
						interactions.push_back(interT);
					}
					string v = (*it2).value; //obtain the value
					//if v is not visited before
					if (visitedEntity.find(v) == visitedEntity.end()) {
						bfs.push(v); //enqueue the string
						visitedEntity.insert(v);  //v has been visited
					}
					++it2; //move to next key
				} while (it2.isValid());
			}
		}
	}
	//sort both vectors
	sort(badEntitiesFound.begin(), badEntitiesFound.end());
	sort(interactions.begin(), interactions.end(), tupleComp);
    return badEntitiesFound.size();
}

bool IntelWeb::purge(const std::string& entity) {
	bool ableToRemove = false;
	//delete nodes where entity is from
	DiskMultiMap::Iterator it1 = datafrom.search(entity);
	while (it1.isValid()) {  //if can find key matching the entity
		ableToRemove = true;  //able to remove
		datafrom.erase((*it1).key, (*it1).value, (*it1).context);//erase this node and same nodes in datafrom
		datato.erase((*it1).value, (*it1).key, (*it1).context);//erase same node in datato
		it1 = datafrom.search(entity);  //find another key
	}
	//delete nodes where entity is to
	DiskMultiMap::Iterator it2 = datato.search(entity);
	//same as previous
	while (it2.isValid()) {
		ableToRemove = true;
		datato.erase((*it2).key, (*it2).value, (*it2).context);
		datafrom.erase((*it2).value, (*it2).key, (*it2).context);
		it2 = datafrom.search(entity);
	}

	return ableToRemove;
}
 //check if the prevalance of certain key is above the threshhold
bool IntelWeb::prevalance(const std::string& key, unsigned int minPrevalenceToBeGood) {
	unsigned int prevalance = 0;
	DiskMultiMap::Iterator it1 = datafrom.search(key); //search the key in first hash table
	if (it1.isValid())
	{
		do
		{
			prevalance++; //for each of the key found, prevalance + 1
			if (prevalance >= minPrevalenceToBeGood)  //if above threshold, return true
				return true;
			++it1; // advance iterator to the next matching item
		} while (it1.isValid());
	}
	//do the same thing for the second hash table
	DiskMultiMap::Iterator it2 = datato.search(key);
	if (it2.isValid())
	{
		do
		{
			prevalance++;
			if (prevalance >= minPrevalenceToBeGood)
				return true;
			++it2; // advance iterator to the next matching item
		} while (it2.isValid());
	}
	return false;
}
