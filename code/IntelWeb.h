#ifndef INTELWEB_H_
#define INTELWEB_H_

#define _CRT_SECURE_NO_DEPRECATE
#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

class IntelWeb
{
public:
        IntelWeb();
        ~IntelWeb();
        bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
        bool openExisting(const std::string& filePrefix);
        void close();
        bool ingest(const std::string& telemetryFile);
        unsigned int crawl(const std::vector<std::string>& indicators,
                	   unsigned int minPrevalenceToBeGood,
                	   std::vector<std::string>& badEntitiesFound,
                	   std::vector<InteractionTuple>& interactions
                	  );
        bool purge(const std::string& entity);

		void print();
private:
	// Your private member declarations will go here
	DiskMultiMap datafrom;  //the from data as key
	DiskMultiMap datato;   // the to data as key
	bool prevalance(const std::string& key, unsigned int minPrevalenceToBeGood); //calculate the prevalance of certain key
};

//inline implementation
//comparison operator for InteractionTuple
inline
bool tupleComp(InteractionTuple a, InteractionTuple b) {
	if (a.context < b.context) {
		return true;
	}
	else if (a.context > b.context) {
		return false;
	}
	else {
		if (a.from < b.from) {
			return true;
		}
		else if (a.from > b.from) {
			return false;
		}
		else {
			if (a.to < b.to) {
				return true;
			}
			else {
				return false;
			}
		}
	}
}

#endif // INTELWEB_H_
                
