
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
		static Countly* m_pInstance;	//单例变量		
		void onStartHelper();  //session初始化助手，记录session开始时间及session开始事件
		void onStopHelper();   //session结束助手，计算sesion持续时间及session结束事件
		void Countly::requestCompleted(HttpClient *sender, HttpResponse *response);   //HTTP请求完成回调事件
		
	public:	  
       static std::mutex mutex;    //多线程同步锁
	   static Countly* GetInstance();  //单例函数

	   /**
	     系统参数初始化
		 @serverURL countly服务器URL
		 @appKey 应用key,由奇酷分配
	   **/
	   void Countly::init(string serverURL, string appKey);

	   /**
	     系统参数初始化
		 @serverURL countly服务器URL
		 @appKey 应用key,由奇酷分配
		 @region 游戏分区
		 @server 游戏服务器
		 @channel 游戏渠道
	   **/ 
	   void Countly::init(string serverURL, string appKey, string region, string server, string channel);	

	   //session事件开始
	   void onStart();

	   //session事件结束
	   void onStop();

	   /**
	    事件记录调用接口
		@key 事件名称
	   **/
	   void recordEvent(string key);

	   /**
	     事件记录调用接口
		 @key 事件名称
		 @count 数量或事件某一属性，如购买的物品个数
	   **/
	   void recordEvent(string key, int count);

	   /**
	     事件记录调用接口
		 @key 事件名称
		 @count 数量或事件某一属性，如购买的物品个数
		 @sum 事件某一属性，如购买的物品价格
	   **/
	   void recordEvent(string key, int count, double sum);

	   /**
	     事件记录调用接口
		 @key 事件名称
		 @segmentation 事件的属性集合
		 @count 数量或事件某一属性，如购买的物品个数		 
	   **/
	   void recordEvent(string key, std::map<std::string, std::string> segmentation, int count);

	   /**
	     事件记录调用接口
		 @key 事件名称
		 @segmentation 事件的属性集合
		 @count 数量或事件某一属性，如购买的物品个数
		 @sum 事件某一属性，如购买的物品价格
	   **/
	   void recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum);

	   //定时器所执行方法，定时更新session状态
	   void onTimer();

	   //将事件数据通过HTTP请求发送到countly服务器
	   void Countly::postThread();
};


class ConnectionQueue
{
	private:	
		//通过后台线程调用Countly::postThread()方法上报事件数据
		void tick();

		std::string serverUrl; //countly服务器URL
        std::string appKey;  // 游戏appKey,由奇酷分配
		std::string region;  // 游戏分区
		std::string server;  // 游戏服务器
		std::string channel;  // 游戏渠道

	public:
		static const int SESSION_DURATION_WHEN_TIME_ADJUSTED = 15;	//默认的session状态更新时间
		void setServerURL(std::string serverUrl);  //设置URL
		std::string getServerURL(); //获取URL
		void setAppKey(std::string appKey);  //设置APPKEY
		std::string getAppKey(); //获取APPKEY
		void setRegion(std::string region);  //设置分区
		std::string getRegion(); //获取分区
		void setServer(std::string server); //设置服务器名称
		std::string getServer(); //获取服务器名称
		void setChannel(std::string channel); //设置渠道
		std::string getChannel(); //获取渠道
		void beginSession(); //session事件开始

		/**
		  session状态更新
		  @duration 持续时间
		**/
		void updateSession(int duration);

		/**
		  session事件结束
		  @duration 持续时间
		**/
		void endSession(int duration);
		
		/**
		  事件记录
		  @events 事件串
		**/
		void recordEvents(string events);

};

class EventQueue
{
    private:
		//转换为十六进制
		unsigned char EventQueue::ToHex(unsigned char x);

		//进行ENCODE编码
		std::string EventQueue::UrlEncode(const std::string& str);
		
		//从十六进制转换
		unsigned char EventQueue::FromHex(unsigned char x);

    public:				
		int EventQueue::size();  //
		string EventQueue::events(); //

		/**
		 事件记录
		 @key 事件名称
		**/
		void EventQueue::recordEvent(string key);

		/**
		 事件记录
		 @key 事件名称
		 @count 数量或事件某一属性，如购买的物品个数
		**/
		void EventQueue::recordEvent(string key, int count);

		/**
		 事件记录
		 @key 事件名称
		 @count 数量或事件某一属性，如购买的物品个数
		 @sum 事件某一属性，如购买的物品价格
		**/
		void EventQueue::recordEvent(string key, int count, double sum);

		/**
		 事件记录
		 @key 事件名称
		 @segmentation 事件的属性集合
		 @count 数量或事件某一属性，如购买的物品个数	
		**/
		void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count);

		/**
		 事件记录
		 @key 事件名称
		 @segmentation 事件的属性集合
		 @count 数量或事件某一属性，如购买的物品个数
		 @sum 事件某一属性，如购买的物品价格
		**/
		void EventQueue::recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum);	
};

class UserEvent
{
	public:
		std::string key;  //事件名称
		std::map<std::string,std::string> segmentation;  //事件属性集合
		int count;  //数量或事件某一属性，如购买的物品个数
        double sum;  //事件某一属性，如购买的物品价格
		int timestamp;  //事件时间
		friend boolean operator == (UserEvent &event1, UserEvent &event2);  //判断两事件是否相同
};

class CountlyStore
{
	private:
		/**
		  字符串切分
		  @str 待切分字符串
		  @separator 分隔符
		**/
		vector<std::string> CountlyStore::split(std::string str,std::string separator);
	public:
		
		CountlyStore();
		vector<std::string> CountlyStore::connections();  //缓存的session事件
		vector<std::string> CountlyStore::events();  //缓存的行为事件
		list<UserEvent> CountlyStore::eventsList();  //缓存的行为事件列表
		boolean CountlyStore::isEmptyConnections();  //是否有缓存的session事件
		boolean CountlyStore::isEmptyEvents();   //是否有缓存的行为事件

		/**
		  从json格式转换为UserEvent对象
		  @json 要转换的json对象
		**/
		UserEvent CountlyStore::jsonToEvent(rapidjson::Document json);

		/**
		  从UserEvent对象转换为json对象
		  @event  要转换的UserEvent对象
		**/
		rapidjson::Document CountlyStore::eventToJSON(UserEvent event);

		/**
		   缓存session事件数据
		   @str session事件数据
		**/
		void CountlyStore::addConnection(std::string str);

		/**
		   移除缓存的session事件数据
		   @str session事件数据
		**/
		void CountlyStore::removeConnection(std::string str);

		/**
		   缓存行为事件数据
		   @event 行为事件
		**/
		void CountlyStore::addEvent(UserEvent event);

		/**
		 缓存行为事件
		 @key 事件名称
		 @segmentation 事件的属性集合
		 @count 数量或事件某一属性，如购买的物品个数
		 @sum 事件某一属性，如购买的物品价格
		**/
		void CountlyStore::addEvent(std::string key, std::map<std::string, std::string> segmentation, int count, double sum);

		/**
		  移除缓存的行为事件
		  @event 行为事件
		**/
		void CountlyStore::removeEvent(UserEvent event);

		/**
		  移除缓存的行为事件集
		  @eventsToRemove 行为事件列表
		**/
		void CountlyStore::removeEvents(list<UserEvent> eventsToRemove);
};



