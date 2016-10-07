#pragma once
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <map>
#include <iostream>

class MumBotRPCWorkQueueMgr {

public:
	int createTextMessageQueue(); //returns index of queue to watch
	void removeTextMessageQueue(int queueid); //deletes queue

	std::string getNextTextMessage(int queueid); //pop next text message in a specific queue
	void pushNextTextMessage(std::string msg); //adds to all queues

private:

	class TextMessageQueue {
	public:
		std::queue<std::string> queue;
		std::mutex cv_mtx;
		std::condition_variable cv;
	};

	std::mutex txtMessageQueuesMutex_; //easiest solution for now
	std::map<int,std::unique_ptr<TextMessageQueue>> textMessageQueues_;

	int maxTextMessageQueueIndex_ = 0;

};
