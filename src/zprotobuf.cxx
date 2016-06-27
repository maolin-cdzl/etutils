#include <assert.h>
#include <google/protobuf/descriptor.h>
#include "etutils/zep/zprotobuf.hpp"

namespace etutils {

google::protobuf::Message* create_message(const google::protobuf::Descriptor* descriptor) {
	if( descriptor ) {
		const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			return prototype->New();
		}
	}
	return nullptr;
}

google::protobuf::Message* create_message(const char* type_name) {
	if( type_name ) {
		const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
		if (descriptor) {
			return create_message(descriptor);
		}
	}
	return nullptr;
}

google::protobuf::Message* build_message(const google::protobuf::Descriptor* descriptor,const void* msg_buf,size_t msg_size) {
	google::protobuf::Message* msg = create_message(descriptor);
	if( msg ) {
		if( msg_buf && msg_size > 0 ) {
			if( !msg->ParseFromArray((const uint8_t*)msg_buf,msg_size) ) {
				delete msg;
				msg = nullptr;
			}
		}
	}
	return msg;
}

google::protobuf::Message* build_message(const char* type_name,const void* msg_buf,size_t msg_size) {
	google::protobuf::Message* msg = create_message(type_name);
	if( msg ) {
		if( msg_buf && msg_size > 0 ) {
			if( !msg->ParseFromArray((const uint8_t*)msg_buf,msg_size) ) {
				delete msg;
				msg = nullptr;
			}
		}
	}
	return msg;
}

int ZProtobuf::send(void* dest,const google::protobuf::Message* msg,bool envelope,int flags) {
	assert(msg);
	assert(dest);

	m_buffer.clear();
	if( ! msg->SerializeToString(&m_buffer) ) {
		return -1;
	}
	
	int moreflags = flags | ZMQ_SNDMORE;
	void* handle = zsock_resolve(dest);

	if( envelope && -1 == zmq_send_const(handle,"",0,moreflags) ) {
		return -1;
	}

	if( -1 == zmq_send_const(handle,msg->GetDescriptor()->full_name().c_str(),msg->GetDescriptor()->full_name().size(),moreflags) ) {
		return -1;
	}

	if( m_buffer.size() != 0 ) {
		return zmq_send(handle,m_buffer.c_str(),m_buffer.size(),flags);
	} else {
		return zmq_send_const(handle,"",0,flags);
	}

}

zmsg_t* ZProtobuf::envelope(void* source) {
	zmsg_t* msg = zmsg_new();
	zframe_t* frame = nullptr;

	do {
		frame = zframe_recv(source);
		if( nullptr == frame ) {
			break;
		}
		if( zframe_size(frame) == 0 ) {
			zmsg_append(msg,&frame);
			break;
		} else {
			zmsg_append(msg,&frame);
		}
	} while(zsock_rcvmore(source));

	return msg;
}

google::protobuf::Message* ZProtobuf::recv(void* source) {
	assert(source);
	char* name = nullptr;
	zframe_t* frame = nullptr;

	// find first no empty frame
	do {
		frame = zframe_recv(source);
		if( nullptr == frame ) {
			break;
		}
		if( zframe_size(frame) != 0 ) {
			break;
		}
		zframe_destroy(&frame);
	} while( zsock_rcvmore(source) );

	if( frame ) {
		name = zframe_strdup(frame);
		if( name && zsock_rcvmore(source)) {
			zframe_t* body = zframe_recv(source);
			if( body ) {
				google::protobuf::Message* msg = build_message(name,zframe_data(body),zframe_size(body));
				zframe_destroy(&body);
				free(name);
				return msg;
			}
		}
	}
	if( name ) {
		free(name);
	}
	if( frame ) {
		zframe_destroy(&frame);
	}
	zsock_flush(source);
	return nullptr;
}

google::protobuf::Message* ZProtobuf::recv(void* source,const google::protobuf::Descriptor* descriptor) {
	assert(source);
	char* name = nullptr;
	zframe_t* frame = nullptr;
	// find first no empty frame
	do {
		frame = zframe_recv(source);
		if( nullptr == frame ) {
			break;
		}
		if( zframe_size(frame) != 0 ) {
			break;
		}
		zframe_destroy(&frame);
	} while( zsock_rcvmore(source) );

	if( frame ) {
		name = zframe_strdup(frame);
		if( name && descriptor->full_name().compare(name) == 0 && zsock_rcvmore(source)) {
			zframe_t* body = zframe_recv(source);
			if( body ) {
				google::protobuf::Message* msg = build_message(descriptor,zframe_data(body),zframe_size(body));
				zframe_destroy(&body);
				free(name);
				return msg;
			}
		}
	}
	if( name ) {
		free(name);
	}
	if( frame ) {
		zframe_destroy(&frame);
	}
	zsock_flush(source);
	return nullptr;
}

}

