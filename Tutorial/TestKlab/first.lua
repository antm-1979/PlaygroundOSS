function setup()
	local pForm = UI_Form(nil,	-- arg[1]:	親となるUIタスクのポインタ
		7000,		-- arg[2]:	基準表示プライオリティ
		0, 0,		-- arg[3,4]:	表示位置
		"asset://first.json",	-- arg[5]:	composit jsonのパス
		false		-- arg[6]:	排他フラグ
	)
	--[[
		arg[6]:排他フラグ は、省略可能です。
		省略した場合は false と同じ挙動になります。
	]]
	
	TASK_StageOnly(pForm)
end

function execute(deltaT)
end

function leave()
	TASK_StageClear()
end


function OnListen()
	NET_listen(9581)
	syslog('----- OnListen() -----')

	sysLoad("asset://MultiImgItem.lua")
end

function OnConnect()
	str = sysCommand(pTB, UI_TEXTBOX_GETTEXT)
	NET_connect(str,9581)

	NET_connect(9581)
	syslog('----- OnConnect() -----')
end
