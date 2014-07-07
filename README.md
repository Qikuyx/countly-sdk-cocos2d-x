countly-sdk-cocos2d-x
=====================
1、	交互协议
使用HTTP协议将用户行为数据上报COUNTLY服务器。

2、主要流程
（1）初始化时可设置服务器URL、appKey、分区、服务器ID、渠道，之后的所有数据上报使用这个配置，推荐使用单例模式；
（2）初始化时启用一个定时器来维护用户的session状态，即用户session状态开始、定时（比如60秒）状态刷新、session结束（用户离线），发送数据分别形如：
app_key=xxxx&device_id=xxxx&region=xxxx&server=xxxx&channel=xxxx&timestamp=xxxx&begin_session=1&metrics=xxxx
app_key=xxxx&device_id=xxxx&region=xxxx&server=xxxx&channel=xxxx&timestamp=xxxx&session_duration=xxxx
app_key=xxxx&device_id=xxxx&region=xxxx&server=xxxx&channel=xxxx&timestamp=xxxx&end_session=1&session_duration=xxxx
（3）所有用户行为数据采用异步发送，避免阻塞正常的用户行为流程，提供多个接口方法供上报调用，形如：
void recordEvent(string key);
void recordEvent(string key, int count);
void recordEvent(string key, int count, double sum);
void recordEvent(string key, std::map<std::string, std::string> segmentation, int count);
void recordEvent(string key, std::map<std::string, std::string> segmentation, int count, double sum);	
多个事件一起上报时，可将其组织为json格式，上报数据形如：
App_key=xxxx&device_id=xxxx&region=xxxx&server=xxxx&channel=xxx&timestamp=xxxx&events=xxxx

