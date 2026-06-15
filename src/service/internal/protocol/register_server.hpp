#pragma once
#include "../../../server_arch/message.hpp"
#include "../../base/def.hpp"

X_SERVICE_BEGIN

struct xMsg_RegisterServer : public xBinaryMessage {  // from proxy_access to relay server
public:
	void SerializeMembers() override {
		W(ServerGroup);		  //
		W(PreviousServerId);  //
		W(ExportAddress);	  //
	}

	void DeserializeMembers() override {
		R(ServerGroup);		  //
		R(PreviousServerId);  //
		R(ExportAddress);	  //
	}

	xServerGroup ServerGroup;
	xServerId	 PreviousServerId;
	xNetAddress	 ExportAddress;
};

struct xMsg_RegisterServerResp : public xBinaryMessage {  // from proxy_access to relay server
public:
	void SerializeMembers() override {
		W(NewServerId);	 //
	}

	void DeserializeMembers() override {
		R(NewServerId);	 //
	}

	xServerId NewServerId;
};

X_SERVICE_END
