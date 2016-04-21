#define _CRT_SECURE_NO_DEPRECATE
#include "DiskMultiMap.h"
#include <functional>
using namespace std;

DiskMultiMap::DiskMultiMap(){} 

DiskMultiMap::~DiskMultiMap(){
	//close all open file
	bf.close();
}

bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets) {
	//if the object has been called to open or create a hash table, close first
	if (bf.isOpen()) {
		bf.close();
	}

	if (bf.createNew(filename.c_str())) {
		bf.write(numBuckets, 0);  //# of buckets, 0 when created
		bf.write(-1, 4);   //tracking the head of removed nodes, initially -1 means no such nodes
		BinaryFile::Offset bucketStart = 8;  //the space for buckets starts at offset 8
		//initialize each bucket with -2
		for (unsigned int i = 0; i < numBuckets; i++) {
			bf.write(-2, bucketStart);  //-2 means the bucket is empty
			bucketStart += 4;  //move to next bucket
		}
		return true;
	}
	else {  //return false if not able to create
		return false;
	}
}

bool DiskMultiMap::openExisting(const std::string& filename) {
	//if the object has been called to open or create a hash table, close first
	if (bf.isOpen()) {
		bf.close();
	}
	//open existing file
	if (bf.openExisting(filename.c_str())) {
		return true;
	}
	else
	{
		return false;//return false if not able to open
	}
}

void DiskMultiMap::close() {
	//only close when there are files already open
	if (bf.isOpen()) {
		bf.close();
	}
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
	//return false if the user tries to insert any key, value or context field with more than 120 or more characters
	if (key.size() > MAX_DATA_LENGTH || value.size() > MAX_DATA_LENGTH || context.size() > MAX_DATA_LENGTH) {
		return false;
	}
	//create the hash value and bucket position for the key
	int numBuckets;
	bf.read(numBuckets, 0);
	hash<string>str_hash;  //built-in hash function return a hash value
	int index = 8 + str_hash(key) % numBuckets*4; //index of the buckets starting at 8

	//check if there is space that can be reused
	//return offest to insert new node
	BinaryFile::Offset newNode;
	BinaryFile::Offset reuse;
	bf.read(reuse, 4);
	if (reuse == -1) { //if no nodes were removed
		newNode = bf.fileLength();  //new location is end of the file
	}
	else {  //if nodes were removed
		newNode = reuse;   //new location is the head of the removed nodes
		Node temp; 
		bf.read(temp, reuse);  //new head of the removed nodes is stored in the next of current node
		reuse = temp.next;
		bf.write(reuse, 4);  //write the new head of the removed nodes into head file
	}
	
	//add the new node into the right location and change corresponding bucket (head)
	BinaryFile::Offset head;
	bf.read(head, index);   //get the current offset of head node for this bucket
	Node in(key.c_str(), value.c_str(), context.c_str(), head); //push current node to the front
	bf.write(in, newNode);   //write the new node into the new location
	head = newNode;
	bf.write(head, index);    //update the head offset
	
	return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
	if (key.size() > MAX_DATA_LENGTH) { //if the length of key is bigger han max size, return invalid iterator
		DiskMultiMap::Iterator it;
		return it;
	}
	//find the bucket index using built-in hash function
	int numBuckets;
	bf.read(numBuckets, 0);
	hash<string>str_hash;  //built-in hash function return a hash value
	int index = 8 + str_hash(key) % numBuckets * 4; //index of the buckets starting at 8
	//read the head of the linked list
	BinaryFile::Offset head;
	bf.read(head, index);
	BinaryFile::Offset node = head;
	//for each node in the linked list
	while (node != -2)
	{
		Node temp;
		if (!bf.read(temp, node)) {  //read the node
			cout << "Error reading from file!\n";
			break;
		}
		else {
			if (strcmp(temp.key, key.c_str()) == 0) {  //check if the key of the node match the target key
				DiskMultiMap::Iterator it(node, this);  //if yes, return iterator pointing to this node
				return it;
			}
		}
		node = temp.next;  //move to next node
	}
	//if not found the node with target key, return empty iterator in invalid state
	DiskMultiMap::Iterator it;
	return it;
}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
	//return 0 if the user tries to insert any key, value or context field with more than 120 or more characters
	if (key.size() > MAX_DATA_LENGTH || value.size() > MAX_DATA_LENGTH || context.size() > MAX_DATA_LENGTH) {
		return 0;
	}
	int numErase = 0;
	//create the bucket index for the key
	
	int numBuckets;
	bf.read(numBuckets, 0);
	hash<string>str_hash;  //built-in hash function return a hash value
	int index = 8 + str_hash(key) % numBuckets * 4; //index of the buckets starting at 8
	BinaryFile::Offset prev = index;  //save the node offset of previous node, initially bucket position
	BinaryFile::Offset node;  
	bf.read(node, index);   //save the node offset of current node, initially the first node

	while (node != -2)  //if not the last node
	{
		Node temp;
		bf.read(temp, node); //read the content of current node
		BinaryFile::Offset nextNodeLoc = temp.next; //the location of next node
		
		//if the value and the context of the current node match
		if (strcmp(temp.key, key.c_str()) == 0 && strcmp(temp.value, value.c_str()) == 0 && strcmp(temp.context, context.c_str()) == 0) {
			bf.write(temp.next, prev);			//previous node point to next node

			//update removed nodes list
			BinaryFile::Offset empty_head;   //the head of the empty nodes list
			bf.read(empty_head, 4);  //read the current head of the empty node list
			temp.next = empty_head;  //connect current removed node with previous removed node
			bf.write(temp, node);  //update the new temp indisk
			empty_head = node;	//put current removed node to the front of removed-node list
			bf.write(empty_head, 4);  //update new empty head
			numErase++;
		}
		//if not match, change previous node to current node
		else {
			prev = node; //prev = the location of "next" member variable in current node
		}

		node = nextNodeLoc; //advance to next node
	}
	
	return numErase;
}

void DiskMultiMap::printAll() {
	/*
	int a;
	char b[121], c[121], d[121];
	char e[1];
	bf.read(a, 776);
	bf.read(b, 121, 412);
	bf.read(c, 121, 533);
	bf.read(d, 122, 654);
	bf.read(e, 1, 775);
	cout << a << " " << b << " " << c <<" "<<d<<" "<<e<< endl;*/

	int numBuckets;
	bf.read(numBuckets, 0);
	BinaryFile::Offset loc = 8;
	for (int i = 0; i < numBuckets; i++) {
		BinaryFile::Offset head;
		bf.read(head, loc);
		BinaryFile::Offset node = head;
		while (node != -2)
		{
			Node temp;
			if (!bf.read(temp, node)) {
				cout << "Error reading from node"<<node<<endl;
				return;
			}
			else {
				cout << node << " ";
				printf(temp.key);
				std::cout << " ";
				printf(temp.value);
				std::cout << " ";
				printf(temp.context);
				std::cout << " ";
				cout << temp.next << endl;
				node = temp.next;
			}
		}
		loc = loc + 4;
	}

}
////////////////////////////////////////////////////////
//create iterator in an invalid state
DiskMultiMap::Iterator::Iterator(): state(false), target(0), parent(nullptr) {}

DiskMultiMap::Iterator::Iterator(BinaryFile::Offset n, DiskMultiMap* d): state(true), target(n), parent(d) {}

bool DiskMultiMap::Iterator::isValid() const {
	return state;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
	//do nothing if the iterator called is invalid
	if (!isValid())
		return *this;

	Node currNode;
	Node nextNode;
	parent->bf.read(currNode, target);  //get the current node
	BinaryFile::Offset node = currNode.next;
	//for each node in the linked list
	while (node != -2) {
		parent->bf.read(nextNode, node); //read next node
		if (strcmp(nextNode.key, currNode.key) == 0) {  //if next node has the same key as current node
			target = node;  //change current iterator to point to next node
			return *this;
		}
		node = nextNode.next;
	}

	//if none of the rest nodes in the same bucket has the same key
	state = false; //set state to be invalid
	return *this;

}

MultiMapTuple DiskMultiMap::Iterator::operator*() {
	MultiMapTuple m;
	Node temp;
	parent->bf.read(temp, target);  //read current node
	//copy data to MultiMapTuple object
	m.key = temp.key;
	m.value = temp.value;
	m.context = temp.context;
	return m;
}