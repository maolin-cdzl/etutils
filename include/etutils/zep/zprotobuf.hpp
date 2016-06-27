#pragma once

#include <string>
#include <google/protobuf/message.h>
#include <czmq.h>

namespace etutils {

google::protobuf::Message* create_message(const google::protobuf::Descriptor* descriptor);
google::protobuf::Message* create_message(const char* type_name);
google::protobuf::Message* build_message(const google::protobuf::Descriptor* descriptor,const void* msg_buf,size_t msg_size);
google::protobuf::Message* build_message(const char* type_name,const void* msg_buf,size_t msg_size);

class ZProtobuf {
public:
	int send(void* dest,const google::protobuf::Message* msg,bool envelope,int flags); 

	// note, envelope(frames end with an empty frame) must exists,or it will read all frames
	// include empty frame
	zmsg_t* envelope(void* source);

	google::protobuf::Message* recv(void* source);
	google::protobuf::Message* recv(void* source,const google::protobuf::Descriptor* descriptor);
private:
	std::string			m_buffer;
};

}


