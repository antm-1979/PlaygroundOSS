#ifndef CKLBLuaLibNET_h
#define CKLBLuaLibNET_h
#include "ILuaFuncLib.h"

class CSockReadStream;
class CKLBLuaLibNET :
	public ILuaFuncLib
{
	static CSockReadStream *m_preadStream;
	//void SetCallBack();
	//const char *m_callback;


	//void SetCallBack(const char *callback);
public:
	CKLBLuaLibNET(DEFCONST * arrConstDef);
	virtual ~CKLBLuaLibNET();
	virtual void addLibrary();

	static s32 luaListen(lua_State * L);
	static s32 luaConnect(lua_State * L);
	static s32 luaRead(lua_State * L);
	static s32 luaWrite(lua_State * L);

};

#endif
