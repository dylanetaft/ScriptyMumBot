#include "MumBotRPCWorkQueueMgr.h"



int MumBotRPCWorkQueueMgr::createTextMessageQueue() {
	std::cout << "Calling createTextMessageQueue\n";
	std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
	std::cout << "Calling createTextMessageQueue - 2\n";
	auto ptr = std::make_unique<TextMessageQueue>();
	textMessageQueues_[maxTextMessageQueueIndex_] = std::move(ptr);
	maxTextMessageQueueIndex_++;
	return maxTextMessageQueueIndex_ - 1;
}

void MumBotRPCWorkQueueMgr::removeTextMessageQueue(int index) {
	std::cout << "Calling removeTextMessageQueue\n";
	std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
	textMessageQueues_.erase(index);
}

void MumBotRPCWorkQueueMgr::pushNextTextMessage(MumBotProto::TextMessage msg) {
	std::cout << "Calling pushNextTextMessage\n";
	std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
	for (const auto &i : textMessageQueues_) { //push to all queues
		std::lock_guard<std::mutex>(i.second->cv_mtx);
		i.second->queue.push(msg);
		i.second->cv.notify_all();
		std::cout << "Notify all\n";
	}
}

MumBotProto::TextMessage MumBotRPCWorkQueueMgr::getNextTextMessage(int queueid) {

	std::cout << "Calling getNextTextMessage\n";
	std::unique_lock<std::mutex> tlock(txtMessageQueuesMutex_);
    try {
        std::unique_ptr<TextMessageQueue> &ptr = textMessageQueues_.at(queueid); 
        tlock.unlock();

        std::unique_lock<std::mutex> qlock(ptr->cv_mtx);

        while (ptr->queue.empty()) {
            ptr->cv.wait(qlock); //will unlock ptr->mtx and wait for notification
        }
        std::cout << "lock 3\n";
        //qlock.lock();
        MumBotProto::TextMessage msg = ptr->queue.front();
        ptr->queue.pop();
        return msg;
    }
    catch (std::exception &e) {
        throw std::out_of_range("MumBotRPCWorkQueueMgr::getNextTextMessage queueid does not exist " + std::to_string(queueid));
    }
    MumBotProto::TextMessage blank;
    return blank;

}
