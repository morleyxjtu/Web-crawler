#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#define _CRT_SECURE_NO_DEPRECATE
#include <string>
#include <cstring>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

const int MAX_DATA_LENGTH = 120;

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		Iterator(BinaryFile::Offset n, DiskMultiMap* d);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
		bool state;
		BinaryFile::Offset target;
		DiskMultiMap* parent;
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

	void printAll();

private:
	// Your private member declarations will go here
	BinaryFile bf;   //Binary file object
	//node storing the data and the next node
	struct Node {
		Node() {} //default constructor
		//constructor to initialize the node
		Node(const char *k, const char *v, const char *c, BinaryFile::Offset n) { 
			strcpy(key, k);
			strcpy(value, v);
			strcpy(context, c);
			next = n;
		}
		BinaryFile::Offset next;  //the offset of next node
		//data
		char key[MAX_DATA_LENGTH+1];
		char value[MAX_DATA_LENGTH+1];
		char context[MAX_DATA_LENGTH+1];  
		
	};

};

#endif // DISKMULTIMAP_H_
