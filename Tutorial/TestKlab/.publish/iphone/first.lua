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
	linkstate =0
end

function execute(deltaT)
	if(linkstate >= 3) then
		linkstate = linkstate - 2
	elseif (linkstate == 1) then
		linkstate = 0
		if (NET_listen(9581) ~= true) then
			sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"listen error,port occypied?Restart system.")
		else
			screen = sysInfo() --send screen size
			NET_writeEvent(2,screen.width * 65536 + screen.height)
			sysLoad("asset://MultiImgItem.lua")
		end
	elseif (linkstate == 2) then
		linkstate = 0
		str = sysCommand(pForm, UI_FORM_UPDATE_NODE,"textbox_target",FORM_TEXT_GET)
		if (NET_connect(str,9581) ~= true) then
			sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"Connect error,ip right?")
		else
			screen = sysInfo()	--send screen size
			NET_writeEvent(2,screen.width * 65536 + screen.height)
			sysLoad("asset://MultiImgItem.lua")
		end
	end

end

function leave()
	TASK_StageClear()
end


function OnListen()
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,"Listen...on port 9581")
	syslog('----- OnListen() -----')
	linkstate = 3
end


function OnConnect()
	str = sysCommand(pForm, UI_FORM_UPDATE_NODE,"textbox_target",FORM_TEXT_GET)
	sysCommand(pForm, UI_FORM_UPDATE_NODE,"label_notify",FORM_TEXT_SET,string.format("Connecting...%s:9581",str))
	syslog('----- OnConnect() -----')
	linkstate = 4
end
