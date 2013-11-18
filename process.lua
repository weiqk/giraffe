require("os")
--local path =  os.getenv("PWD")
--local lualib_path = path..'/lualib/'
require("logging.file")
require("base_type")
require("logic_process")

local zmq = require("zmq")

local ffi = require("ffi")

local trade_time = {"0:00-4:00","5:00-23:59"}
local SLEEP_TIME = 3
local hour_mins = {}

local logger = logging.file("lua_%s.log", "%Y-%m-%d")
--io.write("this is lua script\n")

ffi.cdef [[
#pragma pack(1)

static const int  SHL2_MMP_SIZE=5;

int printf(const char *fmt, ...);

typedef unsigned int time_t;

typedef __int64 int64;

enum DC_GENERAL_INTYPE
{
	GE_IOPV = 5,
	GE_MATU_YLD = 6,
	GE_HKREL_TXT = 1001,
	GE_STATIC_EX = 10001,
	GE_HKDYNA = 10002,
	GE_BLK_STK = 10003,
};

typedef struct UINT24
{
	unsigned short m_wLow;
	char m_cHigh;
}UINT24;

typedef struct MWORD
{
	enum
	{
		MAXBASE = 0x1FFFFFFF,
	};

	int m_nBase:30;
	unsigned int m_nMul:2;
}MWORD;

enum DC_TYPE
{
	DCT_NUL = 0,
	DCT_KEEPALIVE,			//1
	DCT_LOGIN,				//2
	DCT_REQPASS,			//3
	DCT_USERnPASS,			//4
	DCT_READY,				//5
	DCT_RESEND,				//6
	DCT_STKSTATIC,			//7
	DCT_STKDYNA,			//8
	DCT_SHL2_MMPEx,			//9
	DCT_SHL2_REPORT,		//10
	DCT_SHL2_BIG_WD,		//11
	DCT_SHL2_ACCU_WD,		//12
	DCT_HK_STATIC,			//13
	DCT_HK_DYNA,			//14
	DCT_XML,				//15
	DCT_SHL2_QUEUE,			//16
	DCT_GENERAL,			//17
	DCT_USERSTAT,			//18
	DCT_RAWDATA,			//19
	DCT_NEWS,				//20
	DCT_SZL2_ORDER_QUEUE,	//21
	DCT_SZL2_ORDER_STAT,	//22

	DCT_SZL2_ORDER_FIVE=100,
	DCT_SZL2_TRADE_FIVE=101,
};

typedef struct DC_STKSTATIC_MY
{
	unsigned long	m_dwVersion;
	unsigned short	m_wAttrib;		
	unsigned int	m_nDay;			
	short	m_nNum;			
}DC_STKSTATIC_MY;


typedef struct STK_STATIC
{
	unsigned short	id;			
	char	label[10];		
	char	name[32];		
	unsigned char	type;			//STK_TYPE
	unsigned char	price_digit;		
	short	 vol_unit;			
	MWORD	float_issued;		
	MWORD	total_issued;	

	unsigned long	last_close;		
	unsigned long	adv_stop;		
	unsigned long	dec_stop;
}STK_STATIC;

typedef struct DC_STKSTATIC
{
	unsigned long	m_dwVersion;	
	unsigned short	m_wAttrib;		
	unsigned int	m_nDay;			
	short	 m_nNum;			//m_data
	STK_STATIC m_data[1];	
}DC_STKSTATIC;

typedef struct DC_STKDYNA_MY
{
	unsigned short	m_wDynaSeq;		
	short   m_nNum;
}DC_STKDYNA_MY;

typedef struct STK_DYNA
{
	unsigned short	id;			
	time_t	deal_time;				
	unsigned long  open;			
	unsigned long	high;			
	unsigned long	low;			
	unsigned long	last;			
	MWORD	vol;			
	MWORD	amount;			
	MWORD	inner_vol;		
	unsigned long	tick_count;			
	unsigned long	buy_price[5];		
	unsigned long	buy_vol[5];			
	unsigned long	sell_price[5];		
	unsigned long	sell_vol[5];			
	unsigned long	open_interest;		
	unsigned long	settle_price;		
}STK_DYNA;

typedef struct DC_STKDYNA
{
	unsigned short	m_wDynaSeq;		
	short   m_nNum;
	STK_DYNA m_data[1];
}DC_STKDYNA;

typedef struct SHL2_MMPEX
{
	unsigned short	id;			
	unsigned int	avg_buy_price;	
	MWORD	all_buy_vol;		
	unsigned int	avg_sell_price;	
	MWORD	all_sell_vol;		
	unsigned int	buy_price[SHL2_MMP_SIZE];	
	unsigned int	buy_vol[SHL2_MMP_SIZE];		
	unsigned int	sell_price[SHL2_MMP_SIZE];	
	unsigned int	sell_vol[SHL2_MMP_SIZE];		
}SHL2_MMPEX;

typedef struct SHL2_Queue
{
	unsigned short mmp;
}SHL2_Queue;

typedef struct DCS_STKSTATIC_Ex_MY
{
	short	m_nNum;
}DC_STKSTATIC_Ex_MY;


typedef struct STK_STATICEx
{
	enum STK_SUBTYPE
	{
		NILTYPE = 0,
		ASHARE	= 'A',			
		BSHARE	= 'B',			
		GOV_BOND = 'G',			
		ENT_BOND = 'O',			
		FIN_BOND = 'F',			
	};
	enum STK_SP
	{
		NULLSP = 0,
		NSP	= 'N',
		SSP	= 'S',
		PSP = 'P',
		TSP = 'T',
		LSP = 'L',
		OSP = 'O',
		FSP = 'F',
		ESP = 'E',//ETF
		ZSP = 'Z',
	};
	char	m_cType;			
	char	m_cSubType;			//STK_SUBTYPE
	union
	{
		struct 		//(STOCK,OTHER_STOCK)
		{
			float	m_fFaceValue;		
			float	m_fProfit;			
			unsigned short	m_wIndustry;		
			char	m_cTradeStatus;		
			float	m_fCashDividend;
			char	m_cSecurityProperties;
			unsigned long	m_dwLastTradeDate;

		} m_equitySpec;
		 struct 		//,ETF,LOF	(FUND,ETF,LOF)
		{
			float	m_fFaceValue;		
			float	m_fTotalIssued;		
			float	m_fIOPV;			
		} m_fundSpec;
		 struct		//(OPTION,WARRANT)
		{
			char	m_cStyle;			//  'A' or 'E'	American or European Style
			char	m_cCP;				//	'C' or 'P' Call or Put
			float	m_fCnvtRatio;		
			float	m_fStrikePrice;		
			unsigned long	m_dwMaturityDate;	
			char	m_strUnderLine[12];	
			float	m_fBalance;		
		} m_warrantSpec;
		 struct 		//(BOND)
		{
			unsigned long	m_dwMaturityDate;	
			unsigned long	m_dwIntAccruDate;	
			float	m_fIssuePrice;		
			float	m_fCouponRate;		
			float	m_fFaceValue;	
			float	m_fAccruedInt;
		} m_bondSpec;
		 struct		//(COV_BOND)
		{
			char	m_cStyle;			//  'A' or 'E'	American or European Style
			char	m_cCP;				//	'C' or 'P' Call or Put
			float	m_fCnvtRatio;		
			float	m_fStrikePrice;	
			unsigned long	m_dwMaturityDate;	
			char	m_strUnderLine[12];	
			float	m_fAccruedInt;	
		} m_CnvtSpec;
	   struct		//(FUTURE,FTR_IDX,COMM)
		{
			unsigned long	last_day_oi;		
			float	last_settle_price;		
		} m_futureSpec;
		 struct	//(TRUST)
		{
			float	m_dwfAsset;	
			unsigned long	m_dwAssetDate;		//YYYYMMDD
		} m_trustSpec;
	}Spec;
}STK_STATICEx;  

typedef struct STK_HKDYNA
{
	unsigned long	m_dwIEPrice;	
	MWORD	m_mIEVolume;	

	unsigned short	m_wBuyOrderNum[5];
	unsigned short	m_wSellOrderNum[5];

	struct HK_BROKER	
	{
		unsigned short	m_wNumBroker;
		unsigned short	m_wBrokerNo[40];
		char	m_cType[40];		//'B':broker,'R':register trader,'S':number of spread
	}m_buyBroker,m_sellBroker;
}STK_HKDYNA;

typedef struct SZL2_ORDER_STAT
{
	unsigned short m_wStkID;				
	MWORD	m_nBuyOrderVol[4];		
	MWORD	m_nSellOrderVol[4];		
	UINT24	m_dwOrderNum[2];		
	MWORD	m_nWDVol[2];			
}SZL2_ORDER_STAT;

typedef struct SZL2_ORDER_FIVE
{
	unsigned short nIndex;
	char strOrderKind;
	char strFunCode;
	unsigned long dwPrice;
	unsigned long dwAmount;
	unsigned long dwRecNO;
	int   nSetNO;
	unsigned long nRecTime;
}SZL2_ORDER_FIVE;

typedef struct SZL2_TRADE_FIVE
{
	unsigned short nIndex;
	char strOrderKind;
	char strFunCode;
	unsigned long dwPrice;
	unsigned long dwAmount;
	unsigned long dwRecNO;
	int   nSetNO;
	unsigned long   nRecTime;
}SZL2_TRADE_FIVE;

#pragma unpack()
]]

local C = ffi.C
local ffi_cast = ffi.cast 
local str_format = string.format

function FormatReturnError(market_id, dc_type, ret_error)
	local ret_str
	if(ret_error ~= nil) then
		ret_str = str_format("%d:%s:%s\0", market_id, dc_type, ret_error)
	else
		ret_str = nil
	end
	return ret_str
end

function GetZMQPattern(pattern)
	if pattern == 0 then
		return zmq.PAIR
	elseif pattern == 1 then
		return zmq.PUB
	elseif pattern == 2 then
		return zmq.SUB
	elseif pattern == 3 then
		return zmq.REQ
	elseif pattern == 4 then
		return zmq.REP
	elseif pattern == 5 then
		return zmq.DEALER
	elseif pattern == 6 then
		return zmq.ROUTER 
	elseif pattern == 7 then
		return zmq.PULL 
	elseif pattern == 8 then
		return zmq.PUSH 
	elseif pattern == 9 then
		return zmq.XPUB
	elseif pattern == 10 then
		return zmq.XSUB
	else
		return nil
	end
end

local ctx
local sock
local sock_addr
local sock_pattern
local sock_action
function InitZMQ(pattern, action, addr)
	--ctx = zmq.init()
	sock_pattern = GetZMQPattern(pattern)	
	assert(sock_pattern)
	sock_addr = addr
	sock_action = action
	--sock = assert(ctx:socket(sock_pattern))
	--sock_addr = addr
	--if action == "connect" then
	--	assert(sock:connect(sock_addr))		
	--elseif action == "bind" then
	--	assert(sock:bind(sock_addr))
	--end
	InitTradeTime()
end

function InitTradeTime()
	for k,v in ipairs(trade_time) do
		for x in string.gmatch(v,"%d+") do
	        hour_mins[#hour_mins+1] = tonumber(x)
		end                                                                 end
end

function IsTradeTime()
	local now = os.date("*t")
	local now_time = os.time()
	for i=1,table.maxn(hour_mins),4 do
		local ptime = os.time({year=now.year,month=now.month,day=now.day,hour=hour_mins[i],min=hour_mins[i+1],sec=0})
		local ntime = os.time({year=now.year,month=now.month,day=now.day,hour=hour_mins[i+2],min=hour_mins[i+3],sec=0})   
		if (now_time >= ptime and now_time <= ntime) then 
			return true
		end
	end
	return false
end

local last_pack_count = -1
local pack_count = 0
function ObserverOvertime(market_id)
	--zmq.sleep(SLEEP_TIME)
	logger:info("recv overtime from c++")
	if(IsTradeTime()) then
		if(last_pack_count ~= pack_count) then
			last_pack_count = pack_count
		else
			local ret_error = str_format("%d:%d:%s:%s", 0, 1, "not recv data in trading time", "not recv data in trading time for a while")
			local ret_str = FormatReturnError(market_id, "others", ret_error)
			if ret_str ~= nil then 
				SendErrorMsg(ret_str)		
			else
				logger:error("ObserverOvertime ret_str is nil")
			end
		end
	else
		logger:info("not trading time")
	end
end

function SendErrorMsg(error_msg)
	ctx = zmq.init()
	sock = assert(ctx:socket(sock_pattern))
	if sock_action == "connect" then
		assert(sock:connect(sock_addr))		
	elseif sock_action == "bind" then
		assert(sock:bind(sock_addr));
	end

	--local time = os.time()
	local date = os.date()
	local msg = zmq.zmq_msg_t.init_data(error_msg)	
	local send,err = sock:send_msg(msg)
	if send then
		logger:info(date..":send to c++")
	else
		logger:error("send error:",err)
	end
	
	sock:close()
	ctx:term()
end

local did_template_id_table = {}
function init(did_template_id)
	req_id = require(did_template_id)
	table.insert(did_template_id_table,req_id)	
end

function process_basic_type(market_id, dctype, num, pdcdata)
	local stk 
	local dc_type
	local ret_short_error
	local ret_error
	local ret_str 
	pack_count = pack_count + 1
	if dctype == C.DCT_STKSTATIC then
		stk = ffi_cast("STK_STATIC *", pdcdata)
		dc_type = "static"
		logger:info(dc_type)
		for i=1,num do
			ret_error = handle_stk_static(stk)			
			ret_str = FormatReturnError(market_id, dc_type, ret_error)
			--ret_str = "00001:static:0:1:static_error:xxxxxxx\0"
			if ret_str ~= nil then 
				SendErrorMsg(ret_str)		
			end
			stk = stk + 1
		end
	elseif dctype == C.DCT_STKDYNA then
		stk = ffi_cast("STK_DYNA *", pdcdata)
		dc_type = "dyna"
		logger:info(dc_type)
		for i=1,num do
			ret_error = handle_stk_dyna(stk)
			ret_str = FormatReturnError(market_id, dc_type, ret_error)
			--ret_str = "stk-stkdyna\0"
			if ret_str ~= nil then
				SendErrorMsg(ret_str)
			end
			stk = stk + 1
		end
	elseif dctype == C.DCT_SZL2_ORDER_STAT then
		stk = ffi_cast("SZL2_ORDER_STAT *", pdcdata)
		dc_type = "szl2_order_stat"
		for i=1,num do
			ret_error = handle_szl2_order_stat(stk) 
			ret_str = FormatReturnError(market_id, dc_type, ret_error)
			--ret_str = "shl2-order-stat\0"
			if ret_str ~= nil then
				SendErrorMsg(ret_str)
			end
			stk = stk + 1
		end
	elseif dctype == C.DCT_SZL2_TRADE_FIVE then
		stk = ffi_cast("SZL2_TRADE_FIVE*", pdcdata)
		dc_type = "szl2_trade_five\0"
		for i=1,num do
			ret_error = handle_szl2_trade_five(stk)
			ret_str	= FormatReturnError(market_id, dc_type, ret_error)
			--ret_str = dc_type
			if ret_str ~= nil then
				SendErrorMsg(ret_str)
			end
			stk = stk + 1
		end
	elseif dctype == C.DCT_SZL2_ORDER_FIVE then
		stk = ffi_cast("SZL2_ORDER_FIVE*", pdcdata)
		dc_type = "szl2_order_five\0"
		for i=1,num do
			ret_error = handle_szl2_order_five(stk)
			ret_str = FormatReturnError(market_id, dc_type, ret_error)
			ret_str = dc_type
			if ret_str ~= nil then
				SendErrorMsg(ret_str)
			end
			stk = stk + 1
		end
	elseif dctype == C.DCT_SHL2_MMPEx then
		stk = ffi_cast("SHL2_MMPEX *", pdcdata)
		dc_type = "shl2_mmpex"
		for i=1,num do
			ret_error = handle_shl2_mmpex(stk)
			ret_str = FormatReturnError(market_id, dc_type, ret_error)
			--ret_str = "shl2-mmpex\0"
			if ret_str ~= nil then
				SendErrorMsg(ret_str)
			end
			stk = stk + 1
		end
	else
		ret_str = nil
	end
end

function process_did_type(market_id, template_id, num, pdcdata)
	local ret_error
	local ret_str 
	local pdata
	local template_type 
	local template = require(template_id)
	pack_count = pack_count + 1
	logger:info("did")
	if template_id == 100000 then
		pdata = ffi_cast("T_BUY_SELL_INFO *", pdcdata)
		template_type = "t_buy_sell_info"
		for i=1,num do
			ret_error = handle_t_buy_sell_info(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_buy_sell_info\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1		
		end	
	elseif template_id == 100001 then
		pdata = ffi_cast("T_BUY_SELL_TICK_INFO *", pdcdata)
        template_type = "t_buy_sell_tick_info"
        for i=1,num do
            ret_error = handle_t_buy_sell_tick_info(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_buy_sell_tick_info\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1
        end
	elseif template_id == 100002 then
        pdata = ffi_cast("T_IOPV_INFO *", pdcdata)
        template_type = "t_iopv_info"
        for i=1,num do
            ret_error = handle_t_iopv_info(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_iopv_info\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1
        end
    elseif template_id == 100012 then
        pdata = ffi_cast("T_CBT_MARKET *", pdcdata)
        template_type = "t_cbt_market"
        for i=1,num do
            ret_error = handle_t_cbt_market(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_cbt_market\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1
        end
    elseif template_id == 100030 then
        pdata = ffi_cast("T_ETF_INFO *", pdcdata)
        template_type = "t_etf_info"
        for i=1,num do
            ret_error = handle_t_etf_info(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_etf_info\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1
        end
    elseif template_id == 100032 then
        pdata = ffi_cast("T_MMP_INFO *", pdcdata)
        template_type = "t_mmp_info"
        for i=1,num do
            ret_error = handle_t_mmp_info(pdata)
            ret_str = FormatReturnError(market_id, template_type, ret_error)
            --ret_str = "t_mmp_info\0"
            if ret_str ~= nil then
                SendErrorMsg(ret_str)
            end
            pdata = pdata + 1
        end
	else
		ret_str = nil	
	end
end

function process_general_type(intype, num, pdata)
	local stk
	local ret_error
	local ret_str 
	pack_count = pack_count + 1
	logger:info("general")
		for i=1, num do
			if(intype == C.GE_STATIC_EX) then
				stk = ffi_cast("STK_STATICEx *" ,pdata)
				--print(stk.m_cType)
				--print(stk.m_cSubType)
				if(stk.m_cType == 1) then
					dc_type = "ge staticex equity\0"
					ret_error = handle_equity(stk.Spec.m_equitySpec)
					ret_str = FormatReturnError(market_id, dc_type,ret_error)	
					--ret_str = dc_type
					if ret_str ~= nil then
						SendErrorMsg(ret_str)
					end
				elseif(stk.m_cType == 2) then
					dc_type = "ge staticex fund\0"
					ret_error = handle_fund(stk.Spec.m_fundSpec)
					ret_str = FormatReturnError(market_id, dc_type, ret_error)
					--ret_str = dc_type 
					if ret_str ~= nil then
						SendErrorMsg(ret_str)
					end
				elseif(stk.m_cType == 3) then
					dc_type = "ge staticex warrant\0"
					ret_error = handle_fund(stk.Spec.m_warrantSpec)
                    ret_str = FormatReturnError(market_id, dc_type, ret_error)
                    --ret_str = dc_type 
                    if ret_str ~= nil then
                        SendErrorMsg(ret_str)
                    end
					--print("warrantSpec")
					--print(stk.Spec.m_warrantSpec.m_cStyle)
					--print(stk.Spec.m_warrantSpec.m_cCP)
					--print(stk.Spec.m_warrantSpec.m_fStrikePrice)
				elseif(stk.m_cType == 4) then
					dc_type = "ge staticex bond\0"
                    ret_error = handle_fund(stk.Spec.m_bondSpec)
                    ret_str = FormatReturnError(market_id, dc_type, ret_error)
                    --ret_str = dc_type 
                    if ret_str ~= nil then
                        SendErrorMsg(ret_str)
                    end

					--print("bondSpec")
					--print(stk.Spec.m_bondSpec.m_dwMaturityDate)
					--print(stk.Spec.m_bondSpec.m_dwIntAccruDate)
					--print(stk.Spec.m_bondSpec.m_fIssuePrice)
					--print(stk.Spec.m_bondSpec.m_fFaceValue)
				elseif(stk.m_cType == 5) then
					dc_type = "ge staticex Cnvt\0"
					ret_error = handle_fund(stk.Spec.m_CnvtSpec)
                    ret_str = FormatReturnError(market_id, dc_type, ret_error)
                    --ret_str = dc_type
                    if ret_str ~= nil then
                        SendErrorMsg(ret_str)
                    end
				elseif(stk.m_cType == 6) then
					dc_type = "ge staticex future\0"
					ret_error = handle_future(stk.Spec.m_futureSpec)
					ret_str = FormatReturnError(market_id, dc_type,ret_error)
					--ret_str = dc_type
					if ret_str ~= nil then
                        SendErrorMsg(ret_str)
                    end
				elseif(stk.m_cType == 7) then
              	    dc_type = "ge staticex trust\0"
                    ret_error = handle_future(stk.Spec.m_trustSpec)
                    ret_str = FormatReturnError(market_id, dc_type,ret_error)
                    --ret_str = dc_type
                    if ret_str ~= nil then
                        SendErrorMsg(ret_str)
                    end
				else				 
					ret_str = nil
				end 
				stk = stk + 1
			elseif(intype == C.GE_HKDYNA) then
				stk = ffi_cast("STK_HKDYNA *",data)
				dc_type = "ge stk hkdyna\0"
				ret_error = handle_hkdyna(stk)
				ret_str = FormatReturnError(market_id, dc_type, ret_error)
				--ret_str = dc_type 
				if ret_str ~= nil then
					SendErrorMsg(ret_str)
				end
				stk = stk + 1
			elseif(intype == C.GE_IOPV) then
				--stk = ffi_cast("IOPV *",data)
				--dc_type = "ge iopv"
				----ret_error = handle_iopv(stk.value)
				--ret_error = nil
				--ret_str = FormatReturnError(dc_type,ret_error) 
				--ret_str = dc_type
                --if ret_str ~= nil then
                --    SendErrorMsg(ret_str)
                --end
                --stk = stk + 1
			else 
				ret_str = nil
			end
		end
end

--TODO FIX
function process_shl2_queue(dctype, pdcdata)
	local stk
	pack_count = pack_count + 1
	if dctype == C.DCT_SHL2_QUEUE then
		stk = ffi_cast("SHL2_Queue *", pdcdata)
		local ret_error = handle_shl2_mmp(stk)
		dc_type = "shl2_queue"
		local ret_str = FormatReturnError(market_id, dc_type, ret_error)
	end
	return ret_str
end

