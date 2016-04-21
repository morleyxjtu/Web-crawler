//#include "DiskMultiMap.h"
#include "IntelWeb.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main() {
	/*
	//test create, insert
	DiskMultiMap dm;
	dm.createNew("mylist.dat", 100);
	dm.insert("a", "b", "c");
	dm.insert("b", "c", "d");
	dm.insert("e", "c", "d");
	dm.printAll();
	dm.close();*/
	///////////////////////////////////////////////////////////////////////////
	//test search, erase
	/*DiskMultiMap x;
	x.createNew("mylist.dat", 100);; // empty, with 100 buckets
	
	x.insert("hmm.exe", "pfft.exe", "m52902");
	x.insert("hmm.exe", "pfft.exe", "m52902");
	x.insert("hmm.exe", "pfft.exe", "m10001");
	x.insert("blah.exe", "bletch.exe", "m0003");
	cout << "after initial insertion" << endl;
	x.printAll();
	cout << "++++++++++++++++++++++++++++" << endl;

	DiskMultiMap::Iterator it = x.search("hmm.exe");
	if (it.isValid())
	{
		cout << "I found at least 1 item with a key of hmm.exe\n";
		do
		{
			MultiMapTuple m = *it; // get the association
			cout << "The key is: " << m.key << endl;
			cout << "The value is: " << m.value << endl;
			cout << "The context is: " << m.context << endl;
			cout << endl;
			++it; // advance iterator to the next matching item
		} while (it.isValid());
	}

	DiskMultiMap::Iterator it2 = x.search("goober.exe");
	if (!it2.isValid())
		cout << "I couldn find goober.exe\n";

	// line 1
	if (x.erase("hmm.exe", "pfft.exe", "m52902") == 2)
		cout << "Just erased 2 items from the table!\n";
	
	// line 2
	if (x.erase("hmm.exe", "pfft.exe", "m10001") > 0)
		cout << "Just erased at least 1 item from the table!\n";

	// line 3
	if (x.erase("blah.exe", "bletch.exe", "m66666") == 0)
		cout << "I didn't erase this item cause it wasn't there\n";

	cout << "after deletion" << endl;
	x.printAll();
	cout << "++++++++++++++++++++++++++++" << endl;

	x.insert("a.exe", "c.exe", "d");
	x.insert("b.exe", "f.exe", "m52902");
	x.insert("a.exe", "s.exe", "m10001");

	cout << "after second insertion" << endl;
	x.printAll();
	cout << "++++++++++++++++++++++++++++" << endl;*/
	////////////////////////////////////////////////////////////
	
	IntelWeb x;
	x.createNew("mydata", 1000);
	x.close();
	x.openExisting("mydata");

	if (x.ingest("data1.dat"))
		cout << "data1 loaded" << endl; 
	if (x.ingest("data2.dat"))
		cout << "data2 loaded" << endl;
	//x.print();

	vector<string> indicator;
	indicator.push_back("www.virus.com");
	indicator.push_back("a.exe");
	indicator.push_back("www.google.com");
	unsigned int minPrevalenceToBeGood = 10;
	vector<string> badEntitiesFound;
	vector<InteractionTuple> interactions;
	int a = x.crawl(indicator, minPrevalenceToBeGood, badEntitiesFound, interactions);
	for (int i = 0; i < badEntitiesFound.size(); i++) {
		cout << badEntitiesFound[i] << endl;
	}
	for (int i = 0; i < interactions.size(); i++) {
		cout << interactions[i].context <<" "<< interactions[i].from<<" "<< interactions[i].to<<endl;
	}
	cout << "Entity found: "<< a << endl;


	x.purge("a.exe");
	cout << "************************************" << endl;
	x.print();
	cout << "passed" << endl;


	cin.ignore(1000, '/n');
	
}