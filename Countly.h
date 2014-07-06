
#include "cocos2d.h"
#include "../extensions/cocos-ext.h"
#include "network/HttpClient.h"
#include "json/document.h"
#include <iostream>
#include <string>
#include <list>

USING_NS_CC;
using namespace std;  
USING_NS_CC_EXT;
using namespace cocos2d::network;

class Countly
{
    private:	
		Countly();
		~Countly();
		static Countly* m_pInstance;		
		void onHttpRequestCompleted(HttpClient *sender, HttpResponse *response);
		void requestCompleted(HttpClient *sender, HttpResponse *response);
		std::string getcurrentTime();
		void onStart();
		void onStop();
		void onStartHelper();
		void onStopHelper();
		
		
	public:	  
       static std::mutex mutex;
	   static Countly* GetInstance();
	   void Countly::init(string serverURL, string appKey);
	   void Countly::init(string serverURL, string appKey, string region, string server, string channel);
	   void post();
	   void Countly::postThread();
	   void recordEvent(string key);
	   void recordEvent(string key, int count);
	   void recordEvent(string key, int count, double sum);
	   void recordEvent(string key, std::map<std::string, std::string> segmentation, int count);
	   void recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum);	
	   void onTimer();
};


class ConnectionQueue
{
	private:
		std::string appKey_;		
		std::string serverURL_;   
		std::string region_;
		std::string server_;
		std::string channel_;
		void tick();

	public:
		static const int SESSION_DURATION_WHEN_TIME_ADJUSTED = 15;
		std::string getAppKey(){
			return appKey_;
		}

		void setAppKey(string appKey) 
		{
			appKey_ = appKey;
		}
	   
		std::string getServerURL(){
			return serverURL_;
		}

		void setServerURL(string serverURL) 
		{
			serverURL_ = serverURL;
		}
		
		std::string getRegion(){
			return region_;
		}

		void setRegion(string region) {
    		region_ = region;
		}

		std::string getServer(){
			return server_;
		}

		void setServer(string server) {
    		server_ = server;
		}

		std::string getChannel(){
			return channel_;
		}
    
		void setChannel(string channel) {
    		channel_ = channel;
		}

		void beginSession();
		void updateSession(int duration);
		void endSession(int duration);
		void recordEvents(string events);

};

class EventQueue
{
    public:				
		int EventQueue::size();
		string EventQueue::events();
		void EventQueue::recordEvent(string key);
		void EventQueue::recordEvent(string key, int count);
		void EventQueue::recordEvent(string key, int count, double sum);
		void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count);
		void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum);
		unsigned char ToHex(unsigned char x) ;
		unsigned char EventQueue::FromHex(unsigned char x);
		std::string EventQueue::UrlEncode(const std::string& str);
};

class UserEvent
{
	public:
		std::string key;
		std::map<std::string,std::string> segmentation;
		int count;
        double sum;
		int timestamp;
		friend boolean operator == (UserEvent &event1, UserEvent &event2);
};

class CountlyStore
{
	private:
		vector<std::string> CountlyStore::split(std::string str,std::string separator);
	public:
		
		CountlyStore();
		vector<std::string> CountlyStore::connections();
		vector<std::string> CountlyStore::events();
		list<UserEvent> CountlyStore::eventsList();
		boolean CountlyStore::isEmptyConnections();
		boolean CountlyStore::isEmptyEvents();
		UserEvent CountlyStore::jsonToEvent(rapidjson::Document json);
		rapidjson::Document CountlyStore::eventToJSON(UserEvent event);
		void CountlyStore::addConnection(std::string str);
		void CountlyStore::removeConnection(std::string str);
		void CountlyStore::addEvent(UserEvent event);
		void CountlyStore::addEvent(std::string key, std::map<std::string, std::string> segmentation, int count, double sum);
		void CountlyStore::removeEvent(UserEvent event);
		void CountlyStore::removeEvents(list<UserEvent> eventsToRemove);
};



