function setup()
	count = 0
	remoteEvt = 6

	pAssetList = {
      "asset://walk_frame_0001.png.imag",
      "asset://walk_frame_0002.png.imag",
      "asset://walk_frame_0003.png.imag",
      "asset://walk_frame_0004.png.imag",
      "asset://walk_frame_0005.png.imag",
      "asset://walk_frame_0006.png.imag",
      "asset://walk_frame_0007.png.imag",
      "asset://walk_frame_0008.png.imag",
      "asset://walk_frame_0009.png.imag",
      "asset://walk_frame_0010.png.imag",
      "asset://walk_frame_0011.png.imag",
      "asset://walk_frame_0012.png.imag",
      "asset://walk_frame_0013.png.imag",
      "asset://walk_frame_0014.png.imag"
   }
	
	local y1
	local y2
	if (NET_isListen()) then
		y1 = 200
		y2 = 500
	else
		y1 = 500
		y2 = 200
	end
	pMIT = UI_MultiImgItem( nil, 7000, 500, y1, pAssetList, 0)
	local prop = TASK_getProperty(pMIT)
	--prop.color=4278255615
	prop.color=255
	TASK_setProperty(pMIT, prop)		

	picwidth,picheight = ASSET_getImageSize("asset://walk_frame_0001.png.imag")
	orientation = 1		--control people orientation
	bnorit = true		--picture animation start index get from people orientation

	pMITother = UI_MultiImgItem( nil, 7000, 500, y2, pAssetList, 0)
	orientation2 = 1		--control people orientation
	bnorit2 = true		--picture animation start index get from people orientation


	--load arrow
   	pSimpleItem = UI_SimpleItem(	nil,							-- arg[1]:		親となるUIタスクポインタ
									7000,							-- arg[2]:		表示プライオリティ
									0, 0,							-- arg[3,4]:	表示位置
									"asset://arrow.png.imag"	-- arg[5]:		表示assetのパス
								)

   	pTPad = UI_TouchPad("callback_TP")

	screen2 = sysInfo()


	localQueue = {}
	for i=0,11 do
      localQueue[i] =0
    end
	remoteQueue = {}
	for i=0,13 do
      remoteQueue[i] = 0
    end

end


function execute(deltaT)
	--read net event
	local nremote
	repeat
		bhasEvent,event,evtData = NET_readEvent()
		if(bhasEvent ~= true) then break end
		if (event == 1) then			--change orientation
			nremote = remoteEvt % 12
			remoteQueue[nremote] = evtData
			remoteEvt = remoteEvt + 1

			--syslog('NET_readEvent\t')
			--syslog(tostring(event))
			--syslog(tostring(evtData))
		elseif(event == 2 ) then		--get peer screen width and height
			screen2.width = math.floor(evtData / 65536)
			screen2.height = evtData % 65536
			syslog(tostring(screen2.width))
			syslog(tostring(screen2.height))
		end
	until count % 12 == nremote

	if (count + 1) % 12 == remoteEvt % 12 then return end
	count = count + 1

	--send local event to peer
	local nwriteevent = (count + 5) % 12
	NET_writeEvent(1,localQueue[nwriteevent])


	local idx
	local prop
	--local item
	--animation
	local nexec = count % 12
	if (localQueue[nexec] ~= 0) then
		orientation = localQueue[nexec]
		if (orientation == 1) then
			bnorit = true
		elseif (orientation == 3) then
			bnorit = false
		end
		localQueue[nexec] = 0
	end
	if count % 3 == 0 then
		prop = TASK_getProperty(pMIT)
		idx = prop.index
		idx = idx + 1
		idx = idx % 7
		if orientation == 1 then
			sysCommand(pMIT, UI_MULTIIMG_SET_INDEX, idx)
		elseif orienttation == 3 then
			sysCommand(pMIT, UI_MULTIIMG_SET_INDEX, idx+7)
		elseif bnorit == true then
			sysCommand(pMIT, UI_MULTIIMG_SET_INDEX, idx)
		else
			sysCommand(pMIT, UI_MULTIIMG_SET_INDEX, idx+7)
		end
	end
	--move
	prop = TASK_getProperty(pMIT)
	if orientation == 1 then
		prop.x = prop.x + 1
	elseif orientation == 2 then
		prop.y = prop.y + 1
	elseif orientation == 3 then
		prop.x = prop.x - 1
	elseif orientation == 4 then
		prop.y = prop.y - 1
	end
	screen = sysInfo()
	if ( prop.x>=0 and prop.x < screen.width - picwidth and prop.y >= 0 and prop.y < screen.height - picheight ) then
		TASK_setProperty(pMIT, prop)	
	else
		--syslog("Out of screen")
		--syslog(tostring(screen.width))
		--syslog(tostring(screen.height))
		--syslog(tostring(prop.x))
		--syslog(tostring(prop.y))
	end


	--peer item controlled from net
	--animation
	if count % 3 == 0 then
		prop = TASK_getProperty(pMITother)
		idx = prop.index
		idx = idx + 1
		idx = idx % 7
		if orientation2 == 1 then
			sysCommand(pMITother, UI_MULTIIMG_SET_INDEX, idx)
		elseif orienttation2 == 3 then
			sysCommand(pMITother, UI_MULTIIMG_SET_INDEX, idx+7)
		elseif bnorit2 == true then
			sysCommand(pMITother, UI_MULTIIMG_SET_INDEX, idx)
		else
			sysCommand(pMITother, UI_MULTIIMG_SET_INDEX, idx+7)
		end
	end
	--move
	prop = TASK_getProperty(pMITother)
	if orientation2 == 1 then
		prop.x = prop.x + 1
	elseif orientation2 == 2 then
		prop.y = prop.y + 1
	elseif orientation2 == 3 then
		prop.x = prop.x - 1
	elseif orientation2 == 4 then
		prop.y = prop.y - 1
	end
	if (prop.x>=0 and prop.x < screen2.width - picwidth and prop.y >= 0 and prop.y < screen2.height - picheight ) then
		TASK_setProperty(pMITother, prop)	
	end


end


function leave()
end


function callback_TP(tbl)
	local idx
	for idx,item in pairs(tbl) do
		if item.type == PAD_ITEM_TAP then
			--adjust orientation
			if item.x >= 256 then return end
			if item.y >= 256 then return end
			--syslog(string.format("touch postion = %i %i ", item.x,item.y))
			local centx = item.x - 128
			local centy = item.y - 128
			local fabcentx = centx
			local fabcenty = centy
			if fabcentx < 0 then fabcentx = -fabcentx end
			if fabcenty < 0 then fabcenty = -fabcenty end
			local nQueue = (count + 6) % 12
			if centx >= 0 and fabcenty < centx then
				localQueue[nQueue] = 1
			elseif centy >= 0 and centy > fabcentx then
				localQueue[nQueue] = 2
			elseif centx < 0 and fabcenty < -centx then
				localQueue[nQueue] = 3
			elseif centy<0 and -centy > fabcentx then
				localQueue[nQueue] = 4
			end
		end
	end
end
