#include <string>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <memory>
#include <map>
#include <condition_variable>

#include "libmumbot/proto/MumBot.pb.h"
#include "libmumbot/proto/MumBot.grpc.pb.h"


//GRPC includes
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

using namespace libmumbot;

class MumBotRPCWorkQueueMgr {
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
    
    public:
    
    int createTextMessageQueue() { //returns index of queue to watch
        std::cout << "Calling createTextMessageQueue\n";
        std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
        std::cout << "Calling createTextMessageQueue - 2\n";
        auto ptr = std::make_unique<TextMessageQueue>();
        textMessageQueues_[maxTextMessageQueueIndex_] = std::move(ptr);
        maxTextMessageQueueIndex_++;
        return maxTextMessageQueueIndex_ - 1;
    }
    
    void removeTextMessageQueue(int index) { //deletes queue
        std::cout << "Calling removeTextMessageQueue\n";
        std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
        textMessageQueues_.erase(index);
    }
    
    std::string getNextTextMessage(int queueid) { //pop next text message in a specific queue
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
            std::string msg = ptr->queue.front();
            ptr->queue.pop();
            return msg;
        }
        catch (std::exception &e) {
            throw std::out_of_range("MumBotRPCWorkQueueMgr::getNextTextMessage queueid does not exist " + std::to_string(queueid));
        }
        return "";
    
    }
    void pushNextTextMessage(std::string msg) { //adds to all queues
        std::cout << "Calling pushNextTextMessage\n";
        std::lock_guard<std::mutex> lock(txtMessageQueuesMutex_); //easiest solution for now
        for (const auto &i : textMessageQueues_) { //push to all queues
            std::lock_guard<std::mutex>(i.second->cv_mtx);
            i.second->queue.push(msg);
            i.second->cv.notify_all();
            std::cout << "Notify all\n";
        }
    }
    

};


class SegfaultyClass: public protobuf::MumBotRPC::Service {
 
    private:
    MumBotRPCWorkQueueMgr RPCWorkQueueMgr_;
    
    public:
    
    grpc::Status SubscribeToTextMessages(::grpc::ServerContext* context, const protobuf::TextMessageRequest* request, ::grpc::ServerWriter<protobuf::TextMessage>* writer) {
        std::cout << "Client subscribed to text messages\n";
        uint32_t myindex = RPCWorkQueueMgr_.createTextMessageQueue();
        
        for (;;) {
            std::string msg = RPCWorkQueueMgr_.getNextTextMessage(myindex); //will wait
            std::cout << "Got a message:" << msg << "\n";
            if (context->IsCancelled()) break;
            protobuf::TextMessage tmsg;
            tmsg.set_msg(msg);
            writer->Write(tmsg);
        }
        std::cout << "Client connection dropped or cancelled\n";
        RPCWorkQueueMgr_.removeTextMessageQueue(myindex);
        return grpc::Status::OK;
    }
    
    grpc::Status Say(::grpc::ServerContext* context, const protobuf::TextMessage* request, protobuf::TextMessageResponse* response) {
        std::cout << "Message received:" << request->msg() << "\n";
        return grpc::Status::OK;
    }
    
    void startRPCService() {
        grpc::ServerBuilder builder;
        builder.AddListeningPort("0.0.0.0:50080",grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server->Wait();
        std::cout << "Quitting!!!!\n";
    }
    
};

int main() {
    
    SegfaultyClass seggyMcFaultFault;
    seggyMcFaultFault.startRPCService();
    
    return 0;
}

