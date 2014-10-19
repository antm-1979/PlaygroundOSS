function setup()
	pForm = UI_Form(nil,	-- arg[1]:	親となるUIタスクのポインタ
		7000,		-- arg[2]:	基準表示プライオリティ
		0, 0,		-- arg[3,4]:	表示位置
		"asset://first.json",	-- arg[5]:	composit jsonのパス
		false		-- arg[6]:	排他フラグ
	)
	--[[
		arg[6]:排他フラグ は、省略可能です。
		省略した場合は false と同じ挙動になります。
	]]
	
	local hostip = NET_getHostIp()
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_Address",FORM_LBL_SET_TEXT,hostip)
	TASK_StageOnly(pForm)
end

function execute(deltaT)
end

function leave()
	TASK_StageClear()
end


function OnListen()
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"listen...")
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"button_Listen",FORM_NODE_VISIBLE,false)
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"button_Connect",FORM_NODE_VISIBLE,false)
	syslog('----- OnListen() -----')
	if (NET_listen(9581) ~= true) then
		sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"listen error,port occypied?Restart system.")
	else
		screen = sysInfo() --send screen size
		NET_writeEvent(2,screen.width*65536+screen.height)
		sysLoad("asset://MultiImgItem.lua")
	end
end


function OnConnect()
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"connecting...")
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"button_Listen",FORM_NODE_VISIBLE,false)
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"button_Connect",FORM_NODE_VISIBLE,false)
	syslog('----- OnConnect() -----')
	str = sysCommand(pForm, UI_FORM_UPDATE_NODE,"textbox_target",FORM_TEXT_GET)
	if (NET_connect(str,9581) ~= true) then
		sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"connect error,ip correct?")
	else
		screen = sysInfo()	--send screen size
		NET_writeEvent(2,screen.width*65536+screen.height)
		sysLoad("asset://MultiImgItem.lua")
	end
end
