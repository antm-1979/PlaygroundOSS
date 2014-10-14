#include "CKLBLuaLibNET.h"
#include "CSockReadStream.h"
#include "CSockWriteStream.h"
#include "CKLBUtility.h"

static ILuaFuncLib::DEFCONST luaConst[] = {
//	{ "DBG_M_SWITCH",	DBG_MENU::M_SWITCH },
	{ 0, 0 }
};

static CKLBLuaLibNET libdef(luaConst);
CSockReadStream *CKLBLuaLibNET::m_preadStream = nullptr;




CKLBLuaLibNET::CKLBLuaLibNET(DEFCONST * arrConstDef) : ILuaFuncLib(arrConstDef)
{
	//m_preadStream = nullptr;
	//m_callback = nullptr;
}

CKLBLuaLibNET::~CKLBLuaLibNET()
{
	//if (m_callback) { KLBDELETEA(m_callback); }
}

void CKLBLuaLibNET::addLibrary()
{
	addFunction("NET_listen", CKLBLuaLibNET::luaListen);
	addFunction("NET_connect", CKLBLuaLibNET::luaConnect);
	addFunction("NET_readEvent", CKLBLuaLibNET::luaRead);
	addFunction("NET_writeEvent", CKLBLuaLibNET::luaWrite);
}

//void CKLBLuaLibNET::SetCallBack(const char *callback)
//{
//	const char * funcname = lua.getString(2);
//	const char * str = NULL;
//	if (callback) {
//		str = CKLBUtility::copyString(funcname);
//		if (!str) { return false; }
//	}
//	if (m_callback) { KLBDELETEA(m_callback); }
//	m_callback = str;
//}

s32 CKLBLuaLibNET::luaListen(lua_State * L)
{
	CLuaState lua(L);
	int argc = lua.numArgs();
	if (argc != 1) {
		lua.retNil();
		return 1;
	}

	const int port = lua.getInt(1);
	if (!m_preadStream)
		m_preadStream = CSockReadStream::listen(port);

	//const char * callback = lua.getString(1);
	//SetCallBack(callback);

	//lua.retDouble(width);
	//lua.retDouble(height);
	lua.retBoolean(true);
	return 1;
}

s32 CKLBLuaLibNET::luaConnect(lua_State * L)
{
	CLuaState lua(L);
	int argc = lua.numArgs();
	if (argc != 2) {
		lua.retNil();
		return 1;
	}
	const char * ip = lua.getString(1);
	const int port = lua.getInt(2);
	char buf[128];
	sprintf(buf, "%s:%d", ip, port);
	if (!m_preadStream)
		m_preadStream = CSockReadStream::openStream(buf);

	if (m_preadStream->getStatus() != IReadStream::NOT_FOUND)
	{
		//const char * callback = lua.getString(ARG_CALLBACK);
		//SetCallBack(callback);

		lua.retBoolean(true);
		return 1;
	}

	
	//fail operation
	delete m_preadStream;
	m_preadStream = nullptr;

	//lua.retFloat(CKLBNode::s_fBottomBorder);
	//lua.retBoolean(true);
	//lua.retDouble(width);
	//lua.retDouble(height);
	lua.retBoolean(false);
	return 1;
}

s32 CKLBLuaLibNET::luaRead(lua_State * L)
{
	CLuaState lua(L);
	char data[4096];
	unsigned len = m_preadStream->getSize();
	if (m_preadStream->getStatus() == IReadStream::CLOSED)
	{
		lua.retBoolean(false);
		lua.retNil();
		lua.retNil();
		return 3;
	}
	if (len > 4096)
		len = 4096;
	if (len > 0)
	{
		m_preadStream->readBlock(data, len);
	}
	lua.retBoolean(true);
	lua.retInt(*(int *)data);
	lua.retInt(*(int *)(data+4));
	return 3;
}

s32 CKLBLuaLibNET::luaWrite(lua_State * L)
{
	CLuaState lua(L);
	int argc = lua.numArgs();
	if (argc != 2) {
		lua.retNil();
		return 1;
	}
	int evt = lua.getInt(1);
	int evtdata = lua.getInt(2);

	char data[4096];
	*(int *)data = evt;
	*(int *)(data + 4) = evtdata;
	m_preadStream->getWriteStream()->writeBlock(data, 8);
	m_preadStream->getStatus() == IWriteStream::CAN_NOT_WRITE;
	lua.retBoolean(true);
	return 1;
}
