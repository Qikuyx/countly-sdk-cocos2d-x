#include "Countly.h"
#include "../extensions/cocos-ext.h"
#include "network/HttpClient.h"
#include "json/document.h"
#include "cocos2d.h"
#include <iostream>
#include <string>
#include <list>
#include <cstdio>
#include <thread>

USING_NS_CC;
using namespace std;  
USING_NS_CC_EXT;
using namespace cocos2d::network;

Countly* Countly::m_pInstance = new Countly();
ConnectionQueue* queue_;
EventQueue* eventQueue_;
boolean isVisible_;
double unsentSessionLength_;
double lastTime_;
int activityCount_;
CountlyStore countlyStore;
boolean thread_status=false;
static std::mutex mutex;

Countly::Countly()   
{
	queue_ = new ConnectionQueue();	

    isVisible_ = false;
    unsentSessionLength_ = 0;
    activityCount_ = 0;
	CCLOG("%s","construct Countly being called");	

}

Countly::~Countly()
{
}

Countly* Countly::GetInstance()  
{ 		 
	return m_pInstance;  
}  

void Countly::init(string serverUrl, string appKey)   //初始化
{
	//void init(string,string,string,string,string);
	init(serverUrl,appKey,NULL,NULL,NULL);	
}

void Countly::init(string serverURL, string appKey, string region, string server, string channel)  //分区、服务器、渠道初始化
{
	//此处需设置一个定时器执行onTimer方法
	//SetTimer(NULL,1,60000,(TIMERPROC)Countly::TimerProc);
	//Countly::m_pInstance->schedule(schedule_selector(Countly::onTimer), 1);  
	
	queue_->setServerURL(serverURL);
    queue_->setAppKey(appKey);
	if (!region.empty()) {
        queue_->setRegion(region);
    }
    if (!server.empty()) {
        queue_->setServer(server);	
    }
    if (!channel.empty()) {
        queue_->setChannel(channel);
    }
}

void onTimer(){     //定时器所执行方法，定时更新session状态
    if (isVisible_ == false)
        return;
    double currTime = time(NULL);
    unsentSessionLength_ += currTime - lastTime_;
    lastTime_ = currTime;

    int duration = (int) unsentSessionLength_;
    queue_->updateSession(duration);
    unsentSessionLength_ -= duration;

    if (eventQueue_->size() > 0)
        queue_->recordEvents(eventQueue_->events());
}

void onStart() {     //session状态开始
	void onStartHelper();
    activityCount_++;
    if (activityCount_ == 1){
        onStartHelper();
	}
}

void onStop() {    //session状态结束
	void onStopHelper();
    activityCount_--;
    if (activityCount_ == 0){
        onStopHelper();
	}
}

void onStartHelper() 
{	
    lastTime_ = time(NULL);
    queue_->beginSession();
    isVisible_ = true;	
}

void onStopHelper() {	
    if (eventQueue_->size() > 0)
        queue_->recordEvents(eventQueue_->events());

    double currTime = time(NULL);
    unsentSessionLength_ += currTime - lastTime_;

    int duration = (int) unsentSessionLength_;
    queue_->endSession(duration);
    unsentSessionLength_ -= duration;

    isVisible_ = false;	
}

void recordEvent(string key){        //事件上报
	eventQueue_->recordEvent(key);
	if(eventQueue_->size()>10){
		queue_->recordEvents(eventQueue_->events());
	}
}
void recordEvent(string key, int count){   //事件上报
	eventQueue_->recordEvent(key,count);
	if(eventQueue_->size()>10){
		queue_->recordEvents(eventQueue_->events());
	}
}

void recordEvent(string key, int count, double sum) {  //事件上报
	eventQueue_->recordEvent(key,count,sum);
	if(eventQueue_->size()>10){
		queue_->recordEvents(eventQueue_->events());
	}
}

void recordEvent(string key, std::map<std::string, std::string> segmentation, int count){  //事件上报
	eventQueue_->recordEvent(key,segmentation,count);
	if(eventQueue_->size()>10){
		queue_->recordEvents(eventQueue_->events());
	}
}

void recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum){  //事件上报
	eventQueue_->recordEvent(key,segmentation,count,sum);
	if(eventQueue_->size()>10){
		queue_->recordEvents(eventQueue_->events());
	}
}



void ConnectionQueue::beginSession()   //session开始事件
{
	string data;
    data = "app_key=" + queue_->getAppKey();
    data += "&";
	data += "device_id=";
    data += "&";
	data += "region=";
	data += queue_->getRegion();
	data += "&";
	data += "server=" + queue_->getServer();
    data += "&";
	data += "channel=";
	data += queue_->getChannel();
    data += "&";
	data += "timestamp=";
	data += time(NULL);
    data += "&";
	data += "sdk_version=";
	data += "2.0";
    data += "&";
	data += "begin_session=";
	data += "1";
    data += "&";
	data += "metrics=";
    countlyStore.addConnection(data);
    tick();
}

void ConnectionQueue::updateSession(int duration)  //session更新事件
{
	string data;
    data = "app_key=";
	data += queue_->getAppKey();
    data += "&";
	data += "device_id=";	
    data += "&";
	data += "region=";
	data += queue_->getRegion();
    data += "&";
	data += "server=";
	data += queue_->getServer();
    data += "&";
	data += "channel=";
	data += queue_->getChannel();
    data += "&";
	data += "timestamp=";
	data += time(NULL);
    data += "&";
	data += "session_duration=";
	data += (duration > 0 ? duration : SESSION_DURATION_WHEN_TIME_ADJUSTED);

    countlyStore.addConnection(data);

    tick();
}

void ConnectionQueue::endSession(int duration) {    //session结束事件
	string data;
    data = "app_key=" + queue_->getAppKey();
    data += "&";
	data += "device_id=";
    data += "&";
	data += "region=";
	data += queue_->getRegion();
    data += "&";
	data += "server=";
	data += queue_->getServer();
    data += "&";
	data += "channel=";
	data += queue_->getChannel();
    data += "&";
	data += "timestamp=";
	data += time(NULL);
    data += "&";
	data += "end_session=1";
    data += "&";
	data += "session_duration=";
	
	data += (duration > 0 ? duration : SESSION_DURATION_WHEN_TIME_ADJUSTED);

    countlyStore.addConnection(data);

    tick();

}

void ConnectionQueue::recordEvents(string events){    //事件上报数据
	string data;
	data = "app_key=" + queue_->getAppKey();
    data += "&";
	data += "device_id=";
    data += "&";
	data += "region=" + queue_->getRegion();
    data += "&";
	data += "server=";
	data += queue_->getServer();
    data += "&";
	data += "channel=";
	data += queue_->getChannel();
    data += "&";
	data += "timestamp=";
	data += time(NULL);
    data += "&";
	data += "events=";
	data += events;

    countlyStore.addConnection(data);

    tick();
}


void ConnectionQueue::tick(){	//触发事件上报

    if (countlyStore.isEmptyConnections())
        return;
	if(!thread_status){
		std::thread t1(&Countly::postThread);
		t1.detach();
	}
}

void Countly::postThread(void){    //事件上报
	thread_status = true;
	while(true){
		vector<std::string> connections = countlyStore.connections();
        if (connections.size() == 0){
			thread_status = false;
            break;
		}
        string data = connections[0];

		try{
			HttpRequest* request = new HttpRequest();			
			request->setUrl(*queue_->getServerURL().c_str() + "/i?" + *data.c_str());
			request->setRequestType(HttpRequest::Type::POST);
			request->setResponseCallback(CC_CALLBACK_2(Countly::requestCompleted, this));
			request->setTag("countly event");	
			HttpClient::getInstance()->send(request);
			request->release();
			countlyStore.removeConnection(data);
		}
		catch(exception* e){
			thread_status = false;
			break;
		}

	}
}

void Countly::requestCompleted(HttpClient *sender, HttpResponse *response)
{

}

//缓存事件大小
int EventQueue::size(){
	vector<std::string> events = countlyStore.events();
	return events.size();
}

std::string EventQueue::events(){     //缓存的事件
	string result = "";
	Countly::mutex.lock();
	list<UserEvent> events;
	events= countlyStore.eventsList();
	while(!events.empty()){
		result += countlyStore.eventToJSON(events.front()).GetString();	
		events.pop_front();
	}
	countlyStore.removeEvents(events);
	result = EventQueue::UrlEncode(result);
	Countly::mutex.unlock();
	return result;
}

void EventQueue::recordEvent(string key){     //事件放入缓存
	return EventQueue::recordEvent(key,0);
}

void EventQueue::recordEvent(string key, int count){   //事件放入缓存
	return EventQueue::recordEvent(key,count,0.0);
}

void EventQueue::recordEvent(string key, int count, double sum){  //事件放入缓存
	 std::map<std::string,std::string> map;
	 return EventQueue::recordEvent(key,map,count,sum);
}


void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count){   //事件放入缓存
	return EventQueue::recordEvent(key,segmentation,count,0);
}

//事件放入缓存
void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum) {
    
    countlyStore.addEvent(key, segmentation, count, sum);
	//if(eventQueue_->size()>10){
	//	queue_->recordEvents(eventQueue_->events);
	//}
}

unsigned char EventQueue::ToHex(unsigned char x)   
{   
    return  x > 9 ? x + 55 : x + 48;   
}  

unsigned char EventQueue::FromHex(unsigned char x)   
{   
    unsigned char y;  
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;  
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;  
    else if (x >= '0' && x <= '9') y = x - '0';  
    else assert(0);  
    return y;  
}  
  
std::string EventQueue::UrlEncode(const std::string& str)  
{  
    std::string strTemp = "";  
    size_t length = str.length();  
    for (size_t i = 0; i < length; i++)  
    {  
        if (isalnum((unsigned char)str[i]) ||   
            (str[i] == '-') ||  
            (str[i] == '_') ||   
            (str[i] == '.') ||   
            (str[i] == '~'))  
            strTemp += str[i];  
        else if (str[i] == ' ')  
            strTemp += "+";  
        else  
        {  
            strTemp += '%';  
            strTemp += ToHex((unsigned char)str[i] >> 4);  
            strTemp += ToHex((unsigned char)str[i] % 16);  
        }  
    }  
    return strTemp;  
}  



CCUserDefault* userDefault = CCUserDefault::sharedUserDefault(); 
const char* TAG = "COUNTLY_STORE";
const char* PREFERENCES = "COUNTLY_STORE";
const char* DELIMITER = "===";
const char* CONNECTIONS_PREFERENCE = "CONNECTIONS";
const char* EVENTS_PREFERENCE = "EVENTS";
//辅助类，缓存事件
CountlyStore::CountlyStore()
{
	if(!userDefault->getBoolForKey("isExisted"))
	{
		userDefault->setBoolForKey("isExisted",true);
	}
}

//session事件
vector<std::string> CountlyStore::connections()
{  
	std::string array = userDefault->getStringForKey(CONNECTIONS_PREFERENCE,NULL);
	if(!array.empty())
	{
		vector<std::string> connections = CountlyStore::split(array,DELIMITER);
		return connections;
	}
}

//行为事件
vector<std::string> CountlyStore::events()
{
	std::string array = userDefault->getStringForKey(EVENTS_PREFERENCE,NULL);
	if(!array.empty())
	{
		vector<std::string> events = CountlyStore::split(array,DELIMITER);
		return events;
	}
}

//所有行为事件
list<UserEvent> CountlyStore::eventsList()
{
	vector<std::string> eventsVector = CountlyStore::events();
	if(eventsVector.size()==0)
	{
		
	}
	else{
		list<UserEvent> eventsList;
		for(unsigned int i=0;i<eventsVector.size();i++)
		{
			rapidjson::Document d1;
			d1.Parse<0>(eventsVector[i].c_str());
			eventsList.push_back(CountlyStore::jsonToEvent(d1));			
		} 
		return eventsList;
	}
}

//是否有session事件缓存
boolean CountlyStore::isEmptyConnections(){
	vector<std::string> connections = countlyStore.connections();
	if(connections.size()==0){
		return true;
	}
	return false;
}

//是否有行为事件缓存
boolean CountlyStore::isEmptyEvents(){
	if(countlyStore.events().size()==0){
		return true;
	}
	return false;
}

//缓存session事件
void CountlyStore::addConnection(std::string str){
    std::string connections = userDefault->getStringForKey(CONNECTIONS_PREFERENCE,NULL);
	connections += DELIMITER;
	connections += str;
	userDefault->setStringForKey(CONNECTIONS_PREFERENCE,connections);
}
//清除session事件缓存
void CountlyStore::removeConnection(std::string str){
	vector<std::string> connections = countlyStore.connections();
	std::string value;	
	for(vector<std::string>::iterator iter=connections.begin();iter!=connections.end();++iter){
		if(*iter!=str){
			value += *iter;
			value += DELIMITER;
		}
	}	
	userDefault->setStringForKey(CONNECTIONS_PREFERENCE,value);
}
//缓存行为事件
void CountlyStore::addEvent(UserEvent event){
	list<UserEvent> events = countlyStore.eventsList();
	boolean contain = false;
	list<UserEvent>::iterator iter;  
	for(iter = events.begin(); iter!= events.end(); iter++){
		if(*iter==event){
			contain = true;
		}
	}
	if(!contain){
		events.push_back(event);
	}
	std::string value;
	while(!events.empty()){
		value += CountlyStore::eventToJSON(events.front()).GetString();
	}
	userDefault->setStringForKey(EVENTS_PREFERENCE,value);
}

//缓存行为事件
void CountlyStore::addEvent(std::string key, std::map<std::string, std::string> segmentation, int count, double sum) {
	
	list<UserEvent> events = countlyStore.eventsList();
	UserEvent event;	
	while(!events.empty()){
		UserEvent event_ = events.front();
		events.pop_front();
		if(event_.key==key){
			event = event_;
		}	
	}
	if(event.key.empty()){
		event.key = key;
		event.count = count;
		event.sum = sum;
		//event.segmentation=segmentation;
		event.timestamp = time(NULL);
	}
	else{
		CountlyStore::removeEvent(event);
		event.timestamp = time(NULL);
	}
	event.count += count;
	event.sum += sum;
	CountlyStore::addEvent(event);
	
}

//清除行为事件缓存
void CountlyStore::removeEvent(UserEvent event){
	list<UserEvent> events = countlyStore.eventsList();
	events.remove(event);
}
//清除行为事件缓存
void CountlyStore::removeEvents(list<UserEvent> eventsToRemove) {
	list<UserEvent> eventsList = countlyStore.eventsList();
	list<UserEvent>::iterator it;
	for(it=eventsToRemove.begin(); it!=eventsToRemove.end(); ++it){		
		eventsList.remove(*it);
	}
} 

//将json数据转为Event事件
UserEvent CountlyStore::jsonToEvent(rapidjson::Document json)
{
    UserEvent event;
	event.key = json["key"].GetString();
	event.count = json["count"].GetInt();
	event.sum = json["sum"].GetDouble();
	event.timestamp = json["timestamp"].GetInt();
	if(json.HasMember("segmentation")){
		rapidjson::Value &segm = json["segmentation"];
		//if(&segm.IsObject){
		    
			std::map<std::string,std::string> segmentation;
			for (rapidjson::Value::ConstMemberIterator itr = segm.MemberonBegin(); itr != segm.MemberonEnd(); ++itr)
			{				
				segmentation.insert(itr->name.GetString(),itr->value.GetString());
			}
			
		//}		
	}
	return event;
}

//将Event事件转发json格式
rapidjson::Document CountlyStore::eventToJSON(UserEvent event)
{
	rapidjson::Document d;
	d["key"] = event.key;
	d["count"] = event.count;
	d["sum"] = event.count;
	d["timestamp"] = event.timestamp;
	if(!event.segmentation.empty()){
		d["segmentation"] = event.segmentation;
	}
	return d;
}

//事件比较
boolean operator == (UserEvent &event1, UserEvent &event2){
	if(event1.key == event2.key && event1.timestamp == event2.timestamp){
		return true;
	}
	else{
		return false;
	}
}


vector<std::string> CountlyStore::split( std::string str,std::string separator )  
{  
    vector<string> result;  
    int cutAt;  
    while( (cutAt = str.find_first_of(separator)) != str.npos )  
    {  
        if(cutAt > 0)  
        {  
            result.push_back(str.substr(0, cutAt));  
        }  
        str = str.substr(cutAt + 1);  
    }  
    if(str.length() > 0)  
    {  
        result.push_back(str);  
    }        
    return result;  
}  




		
 
