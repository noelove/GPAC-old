/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean le Feuvre
 *			Copyright (c) 2009- Telecom ParisTech
 *			All rights reserved
 *
 *  This file is part of GPAC / Platinum UPnP module 
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 *
 *	----------------------------------------------------------------------------------
 *		PLATINUM IS LICENSED UNDER GPL or commercial agreement - cf platinum license
 *	----------------------------------------------------------------------------------
 *
 */


#include "GPACPlatinum.h"

#ifdef GPAC_HAS_SPIDERMONKEY

#define _SETUP_CLASS(the_class, cname, flag, getp, setp, fin)	\
	memset(&the_class, 0, sizeof(the_class));	\
	the_class.name = cname;	\
	the_class.flags = flag;	\
	the_class.addProperty = JS_PropertyStub;	\
	the_class.delProperty = JS_PropertyStub;	\
	the_class.getProperty = getp;	\
	the_class.setProperty = setp;	\
	the_class.enumerate = JS_EnumerateStub;	\
	the_class.resolve = JS_ResolveStub;		\
	the_class.convert = JS_ConvertStub;		\
	the_class.finalize = fin;

#if !defined(__GNUC__)
# if defined(_WIN32_WCE)
#  pragma comment(lib, "js")
# elif defined (WIN32)
#  pragma comment(lib, "js32")
# endif
#endif

#endif

GF_UPnP::GF_UPnP()
{
	m_pTerm = NULL;
    m_pPlatinum = NULL;
	m_pMediaRenderer = NULL;
	m_pMediaServer = NULL;
	m_pAVCtrlPoint = NULL;
	m_renderer_bound = 0;
	m_pGenericController = NULL;

#ifdef GPAC_HAS_SPIDERMONKEY
	m_Devices = NULL;
	m_pJSCtx = NULL;
	m_nbJSInstances=0;
	last_time = 0;
#endif
}

GF_UPnP::~GF_UPnP()
{
#ifdef GPAC_HAS_SPIDERMONKEY
	if (m_Devices) gf_list_del(m_Devices);
#endif
}

void GF_UPnP::LockTerminal(Bool do_lock)
{
	gf_term_lock_compositor(m_pTerm, do_lock);
}

void GF_UPnP::OnStop(const char *src_url)
{
	if (m_renderer_bound) {
#ifdef GPAC_HAS_SPIDERMONKEY
		jsval funval, rval;
		if (!m_pJSCtx) return;
		LockTerminal(1);
		JS_LookupProperty(m_pJSCtx, m_pObj, "onMediaStop", &funval);
		if (JSVAL_IS_OBJECT(funval)) {
			jsval argv[1];
			argv[0] = GetUPnPDevice(src_url);
			JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 1, argv, &rval);
		}
		LockTerminal(0);
#endif
	} else {
		gf_term_disconnect(m_pTerm);
	}
}

NPT_String GF_UPnP::OnMigrate()
{
	NPT_String res = "";
	if (m_renderer_bound) {
#ifdef GPAC_HAS_SPIDERMONKEY
		jsval funval, rval;
		if (!m_pJSCtx) return res;
		LockTerminal(1);
		JS_LookupProperty(m_pJSCtx, m_pObj, "onMigrate", &funval);
		if (JSVAL_IS_OBJECT(funval)) {
			JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 0, NULL, &rval);
			if (JSVAL_IS_STRING(rval)) {
				res = JS_GetStringBytes(JSVAL_TO_STRING(rval));
			}
		}
		LockTerminal(0);
#endif
	} else {
		GF_NetworkCommand com;

		memset(&com, 0, sizeof(GF_NetworkCommand));
		com.base.command_type = GF_NET_SERVICE_MIGRATION_INFO;
		m_pTerm->root_scene->root_od->net_service->ifce->ServiceCommand(m_pTerm->root_scene->root_od->net_service->ifce, &com);
		if (com.migrate.data) {
			res = com.migrate.data;
		} else {
			res = m_pTerm->root_scene->root_od->net_service->url;
		}
	}
	return res;
}

#ifdef GPAC_HAS_SPIDERMONKEY
jsval GF_UPnP::GetUPnPDevice(const char *src_url) 
{
	return src_url ? STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, src_url ) ) : JSVAL_NULL;
}
#endif

void GF_UPnP::OnConnect(const char *url, const char *src_url)
{
	if (m_renderer_bound) {
#ifdef GPAC_HAS_SPIDERMONKEY
		jsval funval, rval;
		if (!m_pJSCtx) return;
		LockTerminal(1);
		JS_LookupProperty(m_pJSCtx, m_pObj, "onMediaConnect", &funval);
		if (JSVAL_IS_OBJECT(funval)) {
			jsval argv[2];
			argv[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, url ) );
			argv[1] = GetUPnPDevice(src_url);
			JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 2, argv, &rval);
		}
		LockTerminal(0);
#endif
	} else {
		gf_term_navigate_to(m_pTerm, url);
	}
}
void GF_UPnP::OnPause(Bool do_resume, const char *src_url)
{
	if (m_renderer_bound) {
#ifdef GPAC_HAS_SPIDERMONKEY
		jsval funval, rval;
		if (!m_pJSCtx) return;
		LockTerminal(1);
		JS_LookupProperty(m_pJSCtx, m_pObj, do_resume ? "onMediaPlay" : "onMediaPause", &funval);
		if (JSVAL_IS_OBJECT(funval)) {
			jsval argv[1];
			argv[0] = GetUPnPDevice(src_url);
			JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 1, argv, &rval);
		}
		LockTerminal(0);
#endif
	} else {
		gf_term_set_option(m_pTerm, GF_OPT_PLAY_STATE, do_resume ? GF_STATE_PLAYING : GF_STATE_PAUSED);
	}
}

void GF_UPnP::OnSeek(Double time, const char *src_url)
{
	if (m_renderer_bound) {
#ifdef GPAC_HAS_SPIDERMONKEY
		jsval funval, rval;
		if (!m_pJSCtx) return;
		LockTerminal(1);
		JS_LookupProperty(m_pJSCtx, m_pObj, "onSeek", &funval);
		if (JSVAL_IS_OBJECT(funval)) {
			jsval argv[2];
			argv[0] = DOUBLE_TO_JSVAL( JS_NewDouble(m_pJSCtx, time) );
			argv[1] = GetUPnPDevice(src_url);
			JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 2, argv, &rval);
		}
		LockTerminal(0);
#endif
	} else {
		/* CanSeek and Duration set for each media by event_proc */
		if (!m_pTerm->root_scene || (m_pTerm->root_scene->root_od->flags & GF_ODM_NO_TIME_CTRL)
			|| (m_pTerm->root_scene->duration<2000)
		) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_NETWORK, ("[UPnP] Scene not seekable\n"));
		} else {
			gf_term_play_from_time(m_pTerm, (u64) (time * 1000), 0);
		}
	}
}

void GF_UPnP::ContainerChanged(PLT_DeviceDataReference& device, const char *item_id, const char *update_id)
{
}

Bool GF_UPnP::ProcessEvent(GF_Event *evt)
{
	if (!m_pMediaRenderer) return 0;
	switch (evt->type) {
	case GF_EVENT_CONNECT:
		if (evt->connect.is_connected) {
			const char *url = gf_term_get_url(m_pTerm);
			if (url) {
				m_pMediaRenderer->SetConnected(url);
			}
		}
		break;

	case GF_EVENT_DURATION:
			m_pMediaRenderer->SetDuration((int) evt->duration.duration, evt->duration.can_seek);
	case GF_EVENT_METADATA:
		if (m_pTerm->root_scene) {
			char szName[1024];
			const char *artist;
			NetInfoCommand com;
			memset(&com, 0, sizeof(NetInfoCommand));

			/*get any service info*/
			if (gf_term_get_service_info(m_pTerm, m_pTerm->root_scene->root_od, &com) == GF_OK) {
				strcpy(szName, "");
				if (com.name) { strcat(szName, com.name); strcat(szName, " "); }
				if (com.album) { strcat(szName, "("); strcat(szName, com.album); strcat(szName, ")"); }

				artist = "Unknown";
				if (com.artist) artist = com.artist;
				else if (com.writer) artist = com.writer;
				else if (com.composer) artist = com.composer;

				//MRSetMediaInfo(0, szName, com.artist ? com.artist : "Unknown");
			}
		}
		break;
	}
	return 0;
}


void GF_UPnP::Load(GF_Terminal *term)
{
	u16 port = 0;
	Bool save_uuids=0;
	Bool ignore_local_devices=0;
	const char *uuid, *opt, *name;
	char hostname[100], friendly_name[1024];
	GF_LOG(GF_LOG_INFO, GF_LOG_NETWORK, ("[UPnP] Starting UPnP\n"));

	m_pCtrlPoint = NULL;
	m_pTerm = term;

	m_pPlatinum = new PLT_UPnP();
	m_pPlatinum->SetIgnoreLocalUUIDs(false);

	gf_sk_get_host_name((char*)hostname);

	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "AllowedDevicesIP");
	if (!opt) {
		gf_cfg_set_key(m_pTerm->user->config, "UPnP", "AllowedDevicesIP", "");
		opt="";
	}
	m_IPFilter = opt;

	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "IgnoreCreatedDevices");
	if (!opt || !strcmp(opt, "yes")) {
		ignore_local_devices = 1;
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "IgnoreCreatedDevices", "yes");
	}



	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "IgnoreCreatedDevices");
	if (!opt || !strcmp(opt, "yes")) {
		ignore_local_devices = 1;
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "IgnoreCreatedDevices", "yes");
	}


	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "SaveUUIDs");
	if (opt && !strcmp(opt, "yes")) save_uuids = 1;	
	
	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaRendererEnabled");
	if (!opt || !strcmp(opt, "yes")) {
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "MediaRendererEnabled", "yes");

		name = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaRendererName");
		if (!name) {
			sprintf(friendly_name, "GPAC @ %s", hostname);
			name = friendly_name;
		}
		
		port = 0;
		opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaRendererPort");
		if (opt) port = atoi(opt);

		uuid = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaRendererUUID");
		if (uuid) {
			m_pMediaRenderer = new GPAC_MediaRenderer(this, name, false, uuid, port);
		} else {
			m_pMediaRenderer = new GPAC_MediaRenderer(this, name, false, NULL, port);
			if (save_uuids) {
				uuid = m_pMediaRenderer->GetUUID();
				gf_cfg_set_key(m_pTerm->user->config, "UPnP", "MediaRendererUUID", uuid);
			}
		}

		PLT_DeviceHostReference device(m_pMediaRenderer );
		device->m_ModelDescription = "GPAC Media Renderer";
		device->m_ModelURL = "http://gpac.sourceforge.net";
		device->m_ModelNumber = GPAC_FULL_VERSION;
		device->m_ModelName = "GPAC Media Renderer";
		device->m_Manufacturer = "Telecom ParisTech";
		device->m_ManufacturerURL = "http://www.telecom-paristech.fr/";
		m_pPlatinum->AddDevice(device);
	}

	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaServerEnabled");
	if (!opt || !strcmp(opt, "yes")) {
		char *media_root;
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "MediaServerEnabled", "yes");

		name = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaServerName");
		if (!name) {
			sprintf(friendly_name, "GPAC @ %s", hostname);
			name = friendly_name;
		}
		
		port = 0;
		opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaServerPort");
		if (opt) port = atoi(opt);

		uuid = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaServerUUID");
		if (uuid) {
			m_pMediaServer = new GPAC_FileMediaServer(name, false, uuid, port);
		} else {
			m_pMediaServer = new GPAC_FileMediaServer(name, false, NULL, port);
			if (save_uuids) {
				uuid = m_pMediaServer->GetUUID();
				gf_cfg_set_key(m_pTerm->user->config, "UPnP", "MediaServerUUID", uuid);
			}
		}
		media_root = (char *) gf_cfg_get_key(m_pTerm->user->config, "UPnP", "MediaServerRoot");
		if (!media_root) {
			gf_cfg_set_key(m_pTerm->user->config, "UPnP", "MediaServerRoot", "all:/");
			m_pMediaServer->AddSharedDirectory("/", "all");
		} else {
			while (media_root) {
				Bool is_hidden = 0;
				char *sep1 = (char *)strchr(media_root, ':');
				if (!sep1) break;
				char *sep2 = (char *)strchr(sep1, ';');

				if (!strncmp(media_root, "(h)", 3)) {
					media_root+=3;
					is_hidden = 1;
				}
				sep1[0] = 0;
				if (sep2) sep2[0] = 0;
				m_pMediaServer->AddSharedDirectory(sep1+1, media_root, is_hidden);
				sep1[0] = ':';
				if (sep2) sep2[0] = ';';
				else break;
				media_root = sep2+1;
			}
		}
	    PLT_DeviceHostReference device(m_pMediaServer);
		device->m_ModelDescription = "GPAC Media Server";
		device->m_ModelURL = "http://gpac.sourceforge.net";
		device->m_ModelNumber = GPAC_FULL_VERSION;
		device->m_ModelName = "GPAC Media Server";
		device->m_Manufacturer = "Telecom ParisTech";
		device->m_ManufacturerURL = "http://www.telecom-paristech.fr/";
		m_pPlatinum->AddDevice(device);
	}

	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "GenericControllerEnabled");
	if (!opt || !strcmp(opt, "yes")) {
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "GenericControllerEnabled", "yes");
		/*create our generic control point*/
		if (!m_pCtrlPoint) {
			m_pCtrlPoint = new PLT_CtrlPoint();
			m_ctrlPtRef = PLT_CtrlPointReference(m_pCtrlPoint);
		}
		m_pGenericController = new GPAC_GenericController(m_ctrlPtRef, this);
	}

	opt = gf_cfg_get_key(m_pTerm->user->config, "UPnP", "AVCPEnabled");
	if (!opt || !strcmp(opt, "yes")) {
		if (!opt) gf_cfg_set_key(m_pTerm->user->config, "UPnP", "AVCPEnabled", "yes");

		if (!m_pCtrlPoint) {
			m_pCtrlPoint = new PLT_CtrlPoint();
			m_ctrlPtRef = PLT_CtrlPointReference(m_pCtrlPoint);
		}
		m_pAVCtrlPoint = new GPAC_MediaController(m_ctrlPtRef, this);
	}

	// add control point to upnp engine 
	if (m_pCtrlPoint) {
		if (ignore_local_devices) {
			if (m_pMediaServer) m_pCtrlPoint->IgnoreUUID(m_pMediaServer->GetUUID());
			if (m_pMediaRenderer) m_pCtrlPoint->IgnoreUUID(m_pMediaRenderer->GetUUID());
		}
		m_pPlatinum->AddCtrlPoint(m_ctrlPtRef);
	}

	//start UPnP engine
	m_pPlatinum->Start();
	
	/*if we have a control point, force a rescan of the network servcies*/
	if (m_pCtrlPoint) {
		m_pCtrlPoint->Search();
	}
}

void GF_UPnP::Unload()
{
	m_pPlatinum->Stop();
	if (m_pGenericController) delete m_pGenericController;
	if (m_pAVCtrlPoint) delete m_pAVCtrlPoint;
	/*this will delete all UPnP devices*/
	delete m_pPlatinum;
}


#ifdef GPAC_HAS_SPIDERMONKEY

void GF_UPnP::OnMediaRendererAdd(PLT_DeviceDataReference& device, int added)
{
	jsval funval, rval;
	if (!m_pJSCtx) return;

	if (m_IPFilter.GetLength() && (strstr((const char*)m_IPFilter, (const char*)device->GetURLBase().GetHost()) == NULL) ) return;

	LockTerminal(1);

	JS_LookupProperty(m_pJSCtx, m_pObj, "onMediaRendererAdd", &funval);
	if (JSVAL_IS_OBJECT(funval)) {
		jsval argv[3];
		argv[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, device->GetFriendlyName() ) );
		argv[1] = STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, device->GetUUID() ) );
		argv[2] = BOOLEAN_TO_JSVAL( added ? JS_TRUE : JS_FALSE);
		
		JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 3, argv, &rval);
	}
	LockTerminal(0);
}


void GF_UPnP::OnMediaServerAdd(PLT_DeviceDataReference& device, int added)
{
	jsval funval, rval;
	if (!m_pJSCtx) return;

	if (m_IPFilter.GetLength() && (strstr((const char*)m_IPFilter, (const char*)device->GetURLBase().GetHost()) == NULL) ) return;

	LockTerminal(1);
	JS_LookupProperty(m_pJSCtx, m_pObj, "onMediaServerAdd", &funval);
	if (JSVAL_IS_OBJECT(funval)) {
		jsval argv[3];
		argv[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, device->GetFriendlyName() ) );
		argv[1] = STRING_TO_JSVAL( JS_NewStringCopyZ(m_pJSCtx, device->GetUUID() ) );
		argv[2] = BOOLEAN_TO_JSVAL( added ? JS_TRUE : JS_FALSE);
		
		JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 3, argv, &rval);
	}
	LockTerminal(0);
}

static JSBool upnpdevice_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	char *prop_name;
	GPAC_DeviceItem *dev = (GPAC_DeviceItem *)JS_GetPrivate(c, obj);
	if (!dev) return JS_FALSE;

	if (!JSVAL_IS_STRING(id)) return JS_TRUE;
	prop_name = JS_GetStringBytes(JSVAL_TO_STRING(id));
	if (!prop_name) return JS_FALSE;

	if (!strcmp(prop_name, "Name")) {
		*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(c, dev->m_device->GetFriendlyName()) );
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "UUID")) {
		*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(c, dev->m_device->GetUUID()) );
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "PresentationURL")) {
		*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(c, dev->m_device->m_PresentationURL) );
		return JS_TRUE;
	}
	
	return JS_TRUE;
}

static JSBool upnp_device_subscribe(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    PLT_Service* service;
	char *service_uuid = "";
	GPAC_DeviceItem *item = (GPAC_DeviceItem *)JS_GetPrivate(c, obj);
	if (!item || (argc!=2) ) return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0])) return JS_FALSE;
	if (!JSVAL_IS_OBJECT(argv[1])) return JS_FALSE;

	service_uuid = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (item->m_device->FindServiceByType(service_uuid, service) == NPT_SUCCESS) {
		item->m_pUPnP->m_pGenericController->m_CtrlPoint->Subscribe(service);
	}
	return JS_TRUE;
}
static JSBool upnp_device_find_service(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	char *service_uuid = "";
	GPAC_DeviceItem *item = (GPAC_DeviceItem *)JS_GetPrivate(c, obj);
	if (!item || !argc) return JS_FALSE;
	service_uuid = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

	GPAC_ServiceItem *serv = item->FindService(service_uuid);
	if (!serv) {
		*rval = JSVAL_NULL;
		return JS_TRUE;
	}
	*rval = OBJECT_TO_JSVAL(serv->obj);
	return JS_TRUE;
}


void GF_UPnP::OnDeviceAdd(GPAC_DeviceItem *item, int added)
{
	jsval funval, rval;
	if (!m_pJSCtx) return;

	if (m_IPFilter.GetLength() && (strstr((const char*)m_IPFilter, (const char*)item->m_device->GetURLBase().GetHost()) == NULL) ) return;

	LockTerminal(1);

	if (added) {
		item->js_ctx = m_pJSCtx;
		item->obj = JS_NewObject(m_pJSCtx, &upnpGenericDeviceClass, 0, 0);
		item->m_pUPnP = this;
		JS_AddRoot(m_pJSCtx, &item->obj);
		JS_SetPrivate(item->js_ctx, item->obj, item);
	}

	JS_LookupProperty(m_pJSCtx, m_pObj, "onDeviceAdd", &funval);
	if (JSVAL_IS_OBJECT(funval)) {
		jsval argv[2];
		argv[0] = OBJECT_TO_JSVAL( item->obj );
		argv[1] = BOOLEAN_TO_JSVAL( added ? JS_TRUE : JS_FALSE);
		JS_CallFunctionValue(m_pJSCtx, m_pObj, funval, 3, argv, &rval);
	}
	LockTerminal(0);
}

static JSBool upnp_getProperty(JSContext *c, JSObject *obj, jsval id, jsval *vp)
{
	char *prop_name;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp) return JS_FALSE;

	if (!JSVAL_IS_STRING(id)) return JS_TRUE;
	prop_name = JS_GetStringBytes(JSVAL_TO_STRING(id));
	if (!prop_name) return JS_FALSE;

	if (!strcmp(prop_name, "MediaRendererEnabled")) {
		*vp = BOOLEAN_TO_JSVAL( upnp->m_pMediaRenderer ? JS_TRUE : JS_FALSE );
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "MediaServerEnabled")) {
		*vp = BOOLEAN_TO_JSVAL( upnp->m_pMediaServer ? JS_TRUE : JS_FALSE);
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "MediaControlEnabled")) {
		*vp = BOOLEAN_TO_JSVAL( upnp->m_pAVCtrlPoint ? JS_TRUE : JS_FALSE);
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "MediaServersCount")) {
		*vp = INT_TO_JSVAL( upnp->m_pAVCtrlPoint ? gf_list_count(upnp->m_pAVCtrlPoint->m_MediaServers) : 0);
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "MediaRenderersCount")) {
		*vp = INT_TO_JSVAL( upnp->m_pAVCtrlPoint ? gf_list_count(upnp->m_pAVCtrlPoint->m_MediaRenderers) : 0);
		return JS_TRUE;
	}
	if (!strcmp(prop_name, "DevicesCount")) {
		*vp = INT_TO_JSVAL( upnp->m_pGenericController ? gf_list_count(upnp->m_pGenericController->m_Devices) : 0);
		return JS_TRUE;
	}
	return JS_TRUE;
}


static JSBool upnp_get_device(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	u32 idx;
	GPAC_DeviceItem *device;

	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !argc || !JSVAL_IS_INT(argv[0]) ) return JS_FALSE;

	idx = JSVAL_TO_INT(argv[0]);
	if (!upnp->m_pGenericController) return JS_FALSE;

	device = (GPAC_DeviceItem *)gf_list_get(upnp->m_pGenericController->m_Devices, idx);
	if (!device) return JS_FALSE;
	if (!device->obj) {
		device->js_ctx = upnp->m_pJSCtx;
		device->obj = JS_NewObject(upnp->m_pJSCtx, &upnp->upnpGenericDeviceClass, 0, 0);
		device->m_pUPnP = upnp;
		JS_AddRoot(upnp->m_pJSCtx, &device->obj);
		JS_SetPrivate(device->js_ctx, device->obj, device);
	}
	*rval = OBJECT_TO_JSVAL(device->obj);
	return JS_TRUE;
}


static JSBool upnp_find_service(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	const char *dev_ip;
	const char *serv_name;

	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc!=2) || !JSVAL_IS_STRING(argv[0]) || !JSVAL_IS_STRING(argv[1]) ) return JS_FALSE;

	dev_ip = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	serv_name = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	*rval = JSVAL_NULL;
	if (!dev_ip || !serv_name || !upnp->m_pGenericController) return JS_TRUE;

	u32 i, count = gf_list_count(upnp->m_pGenericController->m_Devices);
	for (i=0; i<count; i++) {
		GPAC_DeviceItem *item = (GPAC_DeviceItem *) gf_list_get(upnp->m_pGenericController->m_Devices, i);
		if (item->m_device->GetURLBase().GetHost() == dev_ip) {
			GPAC_ServiceItem *serv = item->FindService(serv_name);
			if (serv) {
				*rval = OBJECT_TO_JSVAL(serv->obj);
				return JS_TRUE;
			}
		}
	}
	return JS_TRUE;
}

static GPAC_MediaRendererItem *upnp_renderer_get_device(GF_UPnP *upnp , JSContext *c, JSObject *obj)
{
	const char *uuid;
	jsval val;
	u32 i, count;
	GPAC_MediaRendererItem *render;
	if (!JS_LookupProperty(c, obj, "UUID", &val) || JSVAL_IS_NULL(val) || JSVAL_IS_VOID(val) )
		return NULL;
	uuid = JS_GetStringBytes(JSVAL_TO_STRING(val));
	if (!uuid) return NULL;

	count = gf_list_count(upnp->m_pAVCtrlPoint->m_MediaRenderers);
	for (i=0; i<count; i++) {
		render = (GPAC_MediaRendererItem *)gf_list_get(upnp->m_pAVCtrlPoint->m_MediaRenderers, i);
		if (render->m_UUID==uuid) return render;
	}
	return NULL;
}

static GPAC_MediaServerItem *upnp_server_get_device(GF_UPnP *upnp , JSContext *c, JSObject *obj)
{
	const char *uuid;
	jsval val;
	u32 i, count;
	GPAC_MediaServerItem *server;
	if (!JS_LookupProperty(c, obj, "UUID", &val) || JSVAL_IS_NULL(val) || JSVAL_IS_VOID(val) )
		return NULL;
	uuid = JS_GetStringBytes(JSVAL_TO_STRING(val));
	if (!uuid) return NULL;

	count = gf_list_count(upnp->m_pAVCtrlPoint->m_MediaServers);
	for (i=0; i<count; i++) {
		server = (GPAC_MediaServerItem *)gf_list_get(upnp->m_pAVCtrlPoint->m_MediaServers, i);
		if (server->m_UUID==uuid) return server;
	}
	return NULL;
}


static JSBool upnp_renderer_open(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *sobj, *fobj;
	jsval val;
	GPAC_MediaRendererItem *render;
	GPAC_MediaServerItem *server;
	const char *item, *resource_url;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc<1) ) return JS_FALSE;

	render = upnp_renderer_get_device(upnp, c, obj);
	if (!render) return JS_TRUE;

	PLT_Service* service;
	server = NULL;
	if (argc==2) {
		sobj = JSVAL_IS_NULL(argv[1]) ? NULL : JSVAL_TO_OBJECT(argv[1]);
		server = sobj ? upnp_server_get_device(upnp, c, sobj) : NULL;
		if (NPT_FAILED(server->m_device->FindServiceByType("urn:schemas-upnp-org:service:ContentDirectory:1", service))) {
			server = NULL;
		}
		if (!server) return JS_TRUE;
	}
	item = NULL;
	resource_url = NULL;
	if (JSVAL_IS_OBJECT(argv[0])) {
		fobj = JSVAL_TO_OBJECT(argv[0]);
		if (!JS_LookupProperty(c, fobj, "ObjectID", &val) || JSVAL_IS_NULL(val) || !JSVAL_IS_STRING(val)) return JS_TRUE;
		item = JS_GetStringBytes(JSVAL_TO_STRING(val));
	}
	else if (JSVAL_IS_STRING(argv[0])) 
		resource_url = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

	if (!item && !resource_url) return JS_TRUE;
	if (item && !server) return JS_TRUE;

    if (NPT_SUCCEEDED(render->m_device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", service))) {
		if (resource_url) {
			upnp->m_pAVCtrlPoint->m_MediaController->SetAVTransportURI(render->m_device, 0, resource_url, NULL, NULL);
			upnp->m_pAVCtrlPoint->m_MediaController->Play(render->m_device, 0, "1", NULL);                        
		} else {
			NPT_String objID = item;

			// look back for the PLT_MediaItem in the results
			PLT_MediaObject* track = NULL;
			NPT_List<PLT_MediaObject*>::Iterator item = server->m_BrowseResults->GetFirstItem();
			while (item) {
				if ((*item)->m_ObjectID == objID) {
					if ((*item)->m_Resources.GetItemCount()) {
						upnp->m_pAVCtrlPoint->m_MediaController->SetAVTransportURI(render->m_device, 0, (*item)->m_Resources[0].m_Uri, (*item)->m_Didl, NULL);
						upnp->m_pAVCtrlPoint->m_MediaController->Play(render->m_device, 0, "1", NULL);                        
					}
					break;
				}
				++item;
			}
		}
	}
	return JS_TRUE;
}

static JSBool upnp_renderer_playback(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval, u32 act_type)
{
	char szSpeed[20];
	GPAC_MediaRendererItem *render;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp) return JS_FALSE;

	render = upnp_renderer_get_device(upnp, c, obj);
	if (!render) return JS_TRUE;

	switch (act_type) {
	/*play/setspeed*/
	case 0:
		strcpy(szSpeed, "1");
		if (argc && JSVAL_IS_NUMBER(argv[0]) ) {
			jsdouble d;
			JS_ValueToNumber(c, argv[0], &d);
			sprintf(szSpeed, "%2.2f", d);
		}
		upnp->m_pAVCtrlPoint->m_MediaController->Play(render->m_device, 0, szSpeed, NULL);                        
		break;
	/*pause*/
	case 1:
		upnp->m_pAVCtrlPoint->m_MediaController->Pause(render->m_device, 0, NULL);                        
		break;
	/*stop*/
	case 2:
		upnp->m_pAVCtrlPoint->m_MediaController->Stop(render->m_device, 0, NULL);                        
		break;
	}
	return JS_TRUE;
}
static JSBool upnp_renderer_play(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return upnp_renderer_playback(c, obj, argc, argv, rval, 0);
}
static JSBool upnp_renderer_pause(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return upnp_renderer_playback(c, obj, argc, argv, rval, 1);
}
static JSBool upnp_renderer_stop(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return upnp_renderer_playback(c, obj, argc, argv, rval, 2);
}

static JSBool upnp_get_renderer(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *s_obj;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pAVCtrlPoint || (argc!=1) ) return JS_FALSE;

	GPAC_MediaRendererItem *mr = NULL;
	if (JSVAL_IS_INT(argv[0])) {
		u32 id = JSVAL_TO_INT(argv[0]);
		mr = (GPAC_MediaRendererItem *) gf_list_get(upnp->m_pAVCtrlPoint->m_MediaRenderers, id);
	}
	else if (JSVAL_IS_STRING(argv[0])) {
		u32 i=0;
		const char *uuid = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
		while ((mr = (GPAC_MediaRendererItem *) gf_list_enum(upnp->m_pAVCtrlPoint->m_MediaRenderers, &i))) {
			if (mr->m_UUID==uuid) break;
		}
	}
	if (!mr) return JS_FALSE;

	s_obj = JS_NewObject(c, &upnp->upnpDeviceClass, 0, 0);
	JS_SetPrivate(c, s_obj, upnp);

	JS_DefineProperty(c, s_obj, "Name", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mr->m_device->GetFriendlyName()) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, s_obj, "UUID", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mr->m_UUID ) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, s_obj, "HostName", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mr->m_device->GetURLBase().GetHost() ) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(c, s_obj, "Open", upnp_renderer_open, 2, 0);
	JS_DefineFunction(c, s_obj, "Play", upnp_renderer_play, 1, 0);
	JS_DefineFunction(c, s_obj, "Pause", upnp_renderer_pause, 0, 0);
	JS_DefineFunction(c, s_obj, "Stop", upnp_renderer_stop, 0, 0);

	*rval = OBJECT_TO_JSVAL(s_obj);
	return JS_TRUE;
}

static JSBool upnp_server_browse(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	NPT_String parent;
	GPAC_MediaServerItem *server;
	const char *dir, *filter;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc!=2) ) return JS_FALSE;

	server = upnp_server_get_device(upnp, c, obj);
	if (!server) return JS_FALSE;

	dir = JSVAL_IS_NULL(argv[0]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (!dir) dir = "0";
	filter = JSVAL_IS_NULL(argv[1]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	if (!filter) filter = "*";

	PLT_Service* service;
	if (NPT_SUCCEEDED(server->m_device->FindServiceByType("urn:schemas-upnp-org:service:ContentDirectory:1", service))) {
		if (!strcmp(dir, "0") || !strcmp(dir, "\\") || !strcmp(dir, "/")) {
			server->m_ParentDirectories.Clear();
		}
		if (!strcmp(dir, "..")) {
			if (!server->m_ParentDirectories.GetItemCount()) return JS_FALSE;
	        server->m_ParentDirectories.Pop(parent);
	        server->m_ParentDirectories.Peek(parent);
			dir=parent;

			if (server->m_ParentDirectories.GetItemCount()==1) 
				server->m_ParentDirectories.Clear();

		} else {
			server->m_ParentDirectories.Push(dir);
		}
		upnp->m_pAVCtrlPoint->Browse(server, dir, filter);


		jsval rval = INT_TO_JSVAL(0);
		if (!server->m_BrowseResults.IsNull()) {
			rval = INT_TO_JSVAL(server->m_BrowseResults->GetItemCount());
		}
		JS_SetProperty(c, obj, "FilesCount", &rval);
	}

	return JS_TRUE;
}


static JSBool upnp_server_has_parent_dir(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_MediaServerItem *server;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp) return JS_FALSE;

	server = upnp_server_get_device(upnp, c, obj);
	if (!server) return JS_TRUE;
	*rval = BOOLEAN_TO_JSVAL( server->m_ParentDirectories.GetItemCount() ? JS_TRUE : JS_FALSE);
	return JS_TRUE;
}


static JSBool upnp_server_get_file(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_MediaServerItem *server;
	u32 id;
	JSObject *f_obj;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc!=1) || !JSVAL_IS_INT(argv[0]) ) return JS_FALSE;

	server = upnp_server_get_device(upnp, c, obj);
	if (!server) return JS_TRUE;
	if (server->m_BrowseResults.IsNull()) return JS_TRUE;

	id = JSVAL_TO_INT(argv[0]);
	PLT_MediaObject *mo;
	server->m_BrowseResults->Get(id, mo);
	if (!mo) return JS_TRUE;

	f_obj = JS_NewObject(c, 0, 0, 0);
	JS_DefineProperty(c, f_obj, "ObjectID", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mo->m_ObjectID)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, f_obj, "Name", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mo->m_Title)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, f_obj, "ParentID", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mo->m_ParentID)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, f_obj, "Directory", BOOLEAN_TO_JSVAL( mo->IsContainer() ? JS_TRUE : JS_FALSE), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	if (!mo->IsContainer() && mo->m_Resources.GetItemCount())
		JS_DefineProperty(c, f_obj, "ResourceURI", STRING_TO_JSVAL( JS_NewStringCopyZ(c, mo->m_Resources[0].m_Uri)) , 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	
	*rval = OBJECT_TO_JSVAL(f_obj);

	return JS_TRUE;
}

static JSBool upnp_server_get_file_uri(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_MediaServerItem *server;
	u32 id;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc!=1) || !JSVAL_IS_INT(argv[0]) ) return JS_FALSE;

	server = upnp_server_get_device(upnp, c, obj);
	if (!server) return JS_TRUE;
	if (server->m_BrowseResults.IsNull()) return JS_TRUE;

	id = JSVAL_TO_INT(argv[0]);
	PLT_MediaObject *mo;
	server->m_BrowseResults->Get(id, mo);
	if (!mo) return JS_TRUE;

	if (mo->m_Resources.GetItemCount()) {
		*rval = STRING_TO_JSVAL( JS_NewStringCopyZ(c, mo->m_Resources[0].m_Uri) );
	}
	return JS_TRUE;
}

static JSBool upnp_get_server(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *s_obj;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pAVCtrlPoint || (argc!=1)) return JS_FALSE;


	GPAC_MediaServerItem *ms = NULL;
	if (JSVAL_IS_INT(argv[0])) {
		u32 id = JSVAL_TO_INT(argv[0]);
		ms = (GPAC_MediaServerItem *) gf_list_get(upnp->m_pAVCtrlPoint->m_MediaServers, id);
	}
	else if (JSVAL_IS_STRING(argv[0])) {
		u32 i=0;
		const char *uuid = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
		while ((ms = (GPAC_MediaServerItem *) gf_list_enum(upnp->m_pAVCtrlPoint->m_MediaServers, &i))) {
			if (ms->m_UUID==uuid) break;
		}
	}
	if (!ms) return JS_FALSE;
	s_obj = JS_NewObject(c, &upnp->upnpDeviceClass, 0, 0);
	JS_SetPrivate(c, s_obj, upnp);

	JS_DefineProperty(c, s_obj, "Name", STRING_TO_JSVAL( JS_NewStringCopyZ(c, ms->m_device->GetFriendlyName()) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, s_obj, "UUID", STRING_TO_JSVAL( JS_NewStringCopyZ(c, ms->m_UUID ) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(c, s_obj, "FilesCount", INT_TO_JSVAL(10), 0, 0, 0  | JSPROP_PERMANENT);
	JS_DefineFunction(c, s_obj, "Browse", upnp_server_browse, 2, 0);
	JS_DefineFunction(c, s_obj, "GetFile", upnp_server_get_file, 1, 0);
	JS_DefineFunction(c, s_obj, "GetFileURI", upnp_server_get_file_uri, 1, 0);
	JS_DefineFunction(c, s_obj, "HasParentDirectory", upnp_server_has_parent_dir, 0, 0);


	*rval = OBJECT_TO_JSVAL(s_obj);
	return JS_TRUE;
}

static JSBool upnp_bind_renderer(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp) return JS_TRUE;
	upnp->m_renderer_bound = 1;

	/*remove ourselves from the event filters since we will only be called through JS*/
	gf_term_unregister_event_filter(upnp->m_pTerm, upnp->term_ext);

	return JS_TRUE;
}
static JSBool upnp_set_url(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	const char *url;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pMediaRenderer || !argc || !JSVAL_IS_STRING(argv[0]) ) return JS_TRUE;
	url = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (url) upnp->m_pMediaRenderer->SetConnected(url);
	return JS_TRUE;
}
static JSBool upnp_set_duration(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsdouble d;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pMediaRenderer || !argc || !JSVAL_IS_DOUBLE(argv[0]) ) return JS_TRUE;
	JS_ValueToNumber(c, *rval, &d);
	upnp->m_pMediaRenderer->SetDuration((u32) d, 1);
	return JS_TRUE;
}

static JSBool upnp_share_resource(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	const char *url, *host;
	NPT_String resourceURI;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pMediaServer || !argc || !JSVAL_IS_STRING(argv[0]) ) return JS_TRUE;
	url = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (!url) return JS_TRUE;

	host = NULL;
	if (argc && JSVAL_IS_STRING(argv[1]) ) {
		host = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	}

	resourceURI = upnp->m_pMediaServer->GetResourceURI(url, host);
	*rval = STRING_TO_JSVAL( JS_NewStringCopyZ(upnp->m_pJSCtx, resourceURI ) );
	return JS_TRUE;
}

static JSBool upnp_share_virtual_resource(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	Bool temp = 0;
	const char *res_url, *res_val, *mime;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || !upnp->m_pMediaServer || (argc<2) || !JSVAL_IS_STRING(argv[0]) || !JSVAL_IS_STRING(argv[1]) ) return JS_TRUE;
	res_url = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (!res_url) return JS_TRUE;
	res_val = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	if (!res_val) return JS_TRUE;

	mime = "application/octet-stream";
	if (argc==3) mime = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
	if ((argc==4) && JSVAL_IS_BOOLEAN(argv[3]) && (JSVAL_TO_BOOLEAN(argv[3])==JS_TRUE) ) temp = 1;;

	upnp->m_pMediaServer->ShareVirtualResource(res_url, res_val, mime, temp);
	return JS_TRUE;
}


static NPT_UInt8 GENERIC_SCPDXML[] = "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"><specVersion>  <major>1</major>   <minor>0</minor> </specVersion> <actionList>  <action>  <name>GetStatus</name>    <argumentList>    <argument>     <name>ResultStatus</name>     <direction>out</direction>      <relatedStateVariable>Status</relatedStateVariable>     </argument>   </argumentList>  </action> </actionList>  <serviceStateTable>  <stateVariable sendEvents=\"yes\">   <name>Status</name>    <dataType>boolean</dataType>   </stateVariable></serviceStateTable> </scpd>";


static JSBool upnp_device_setup_service(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	char *name, *type, *id, *scpd_xml;
	GPAC_GenericDevice *device = (GPAC_GenericDevice *)JS_GetPrivate(c, obj);
	if (!device) return JS_FALSE;
	if (argc<3) return JS_FALSE;

	name = NULL;
	if (JSVAL_IS_STRING(argv[0])) name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	if (!name) return JS_FALSE;

	type = NULL;
	if (JSVAL_IS_STRING(argv[1])) type = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	if (!type) return JS_FALSE;

	id = NULL;
	if (JSVAL_IS_STRING(argv[2])) id = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
	if (!id) return JS_FALSE;

	scpd_xml = NULL;
	if ((argc>3) && JSVAL_IS_STRING(argv[3])) scpd_xml = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));
	if (!scpd_xml) scpd_xml = (char *)GENERIC_SCPDXML;


	GPAC_Service* service = new GPAC_Service(device, type, id);
	if (service->SetSCPDXML((const char*)scpd_xml) != NPT_SUCCESS) {
		delete service;
		return JS_FALSE;
	}
	if (service->InitURLs(name, device->GetUUID() ) != NPT_SUCCESS) {
		delete service;
		return JS_FALSE;
	}
	gf_list_add(device->m_pServices, service);

	service->SetupJS(c, device->m_pUPnP, device->obj);
	*rval = OBJECT_TO_JSVAL(service->m_pObj);
	return JS_TRUE;
}



static JSBool upnp_device_start(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsval sval;
	char *str;
	GPAC_GenericDevice *device = (GPAC_GenericDevice *)JS_GetPrivate(c, obj);
	if (!device) return JS_FALSE;

	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "PresentationURL", &sval) && JSVAL_IS_STRING(sval)) {
		str = JS_GetStringBytes(JSVAL_TO_STRING(sval));
		char *url = gf_url_concatenate(device->js_source, str);

		/*we will use our media server to exchange the URL if file based
			!!! THIS IS BROKEN IF MULTIPLE INTERFACES EXIST ON THE DEVICE !!!
		*/
		if (device->m_pUPnP->m_pMediaServer) {
			device->m_PresentationURL = device->m_pUPnP->m_pMediaServer->GetResourceURI(url, NULL);
		}
		/*otherwise we can only use absolute URLs */
		else if (strstr(url, "://") && !strstr(url, "file://")) {
			device->m_PresentationURL = url;
		}
		gf_free(url);
	}

	str = NULL;
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "ModelDescription", &sval) && JSVAL_IS_STRING(sval))
		str = JS_GetStringBytes(JSVAL_TO_STRING(sval));
	device->m_ModelDescription = str ? str : "GPAC Generic Device";

	str = NULL;
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "ModelURL", &sval) && JSVAL_IS_STRING(sval))
		str = JS_GetStringBytes(JSVAL_TO_STRING(sval));
	device->m_ModelURL = str ? str : "http://gpac.sourceforge.net";

	str = NULL;
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "ModelNumber", &sval) && JSVAL_IS_STRING(sval))
		str = JS_GetStringBytes(JSVAL_TO_STRING(sval));
	device->m_ModelNumber = str ? str : GPAC_FULL_VERSION;

	str = NULL;
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "ModelName", &sval) && JSVAL_IS_STRING(sval))
		str = JS_GetStringBytes(JSVAL_TO_STRING(sval));
	device->m_ModelName = str ? str : "GPAC Generic Device";

	device->m_Manufacturer = "Telecom ParisTech";
	device->m_ManufacturerURL = "http://www.telecom-paristech.fr/";

	if (device->m_pUPnP->m_pGenericController) {
		const char *opt = gf_cfg_get_key(device->m_pUPnP->m_pTerm->user->config, "UPnP", "IgnoreCreatedDevices");
		if (!opt || !strcmp(opt, "yes")) {
			device->m_pUPnP->m_pGenericController->m_CtrlPoint->IgnoreUUID(device->GetUUID());
		}
	}
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "Run", &sval) && JSVAL_IS_OBJECT(sval)) {
		device->obj = obj;
		device->run_proc = sval;
		JS_AddRoot(device->m_pUPnP->m_pJSCtx, &device->run_proc);
	}
	if (JS_LookupProperty(device->m_pUPnP->m_pJSCtx, obj, "OnAction", &sval) && JSVAL_IS_OBJECT(sval)) {
		device->obj = obj;
		device->act_proc = sval;
		JS_AddRoot(device->m_pUPnP->m_pJSCtx, &device->act_proc);
	}
	PLT_DeviceHostReference devRef(device);
	device->m_pUPnP->m_pPlatinum->AddDevice(devRef);

	return JS_TRUE;
}

static JSBool upnp_device_stop(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_GenericDevice *device = (GPAC_GenericDevice *)JS_GetPrivate(c, obj);
	if (!device) return JS_FALSE;

	PLT_DeviceHostReference devRef(device);
	device->m_pUPnP->m_pPlatinum->RemoveDevice(devRef);

	return JS_TRUE;
}


static GPAC_GenericDevice *upnp_create_generic_device(GF_UPnP *upnp, JSObject*global, const char *id, const char *name)
{
	GPAC_GenericDevice *device;
	device = new GPAC_GenericDevice(name, id);
	device->m_pUPnP = upnp;
	device->js_source = "";

	device->obj = JS_NewObject(upnp->m_pJSCtx, &upnp->upnpDeviceClass, 0, global);
	JS_AddRoot(upnp->m_pJSCtx, &device->obj);

	JS_DefineProperty(upnp->m_pJSCtx, device->obj, "Name", STRING_TO_JSVAL( JS_NewStringCopyZ(upnp->m_pJSCtx, name) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(upnp->m_pJSCtx, device->obj, "ID", STRING_TO_JSVAL( JS_NewStringCopyZ(upnp->m_pJSCtx, id) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(upnp->m_pJSCtx, device->obj, "UUID", STRING_TO_JSVAL( JS_NewStringCopyZ(upnp->m_pJSCtx, device->GetUUID()) ), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(upnp->m_pJSCtx, device->obj, "SetupService", upnp_device_setup_service, 0, 0);
	JS_DefineFunction(upnp->m_pJSCtx, device->obj, "Start", upnp_device_start, 0, 0);
//	JS_DefineFunction(upnp->m_pJSCtx, device->obj, "Stop", upnp_device_stop, 0, 0);
	JS_SetPrivate(upnp->m_pJSCtx, device->obj, device);
	if (!upnp->m_Devices) upnp->m_Devices = gf_list_new();
	gf_list_add(upnp->m_Devices, device);

	return device;
}

static JSBool upnp_create_device(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_GenericDevice *device;
	const char *id, *name;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc != 2)) return JS_FALSE;

	id = NULL;
	if (JSVAL_IS_STRING(argv[0])) id = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	name = NULL;
	if (JSVAL_IS_STRING(argv[1])) name = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	if (!id || !name) return JS_FALSE;

	device = upnp_create_generic_device(upnp, NULL, id, name);
	if (device)
		*rval = OBJECT_TO_JSVAL(device->obj);

	return JS_TRUE;
}

static JSBool upnp_delete_device(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	GPAC_GenericDevice *device;
	GF_UPnP *upnp = (GF_UPnP *)JS_GetPrivate(c, obj);
	if (!upnp || (argc != 1)) return JS_FALSE;

	device = (GPAC_GenericDevice *) JS_GetPrivate(c, JSVAL_TO_OBJECT(argv[0]) );
	if (!device) return JS_FALSE;

	gf_list_del_item(upnp->m_Devices, device);
	device->DetachJS(c);
	PLT_DeviceHostReference devRef = PLT_DeviceHostReference(device);
	upnp->m_pPlatinum->RemoveDevice(devRef);
	devRef.Detach();
	return JS_TRUE;
}

Bool GF_UPnP::LoadJS(GF_TermExtJS *param)
{
	u32 i, count;
	JSPropertySpec upnpClassProps[] = {
		{0, 0, 0, 0, 0}
	};
	JSFunctionSpec upnpClassFuncs[] = {
		{"BindRenderer", upnp_bind_renderer, 0, 0, 0},
		{"GetMediaServer", upnp_get_server, 1, 0, 0},
		{"GetMediaRenderer", upnp_get_renderer, 1, 0, 0},
		{"SetCurrentURL", upnp_set_url, 1, 0, 0},
		{"SetDuration", upnp_set_duration, 1, 0, 0},
		{"ShareResource", upnp_share_resource, 1, 0, 0},	
		{"ShareVirtualResource", upnp_share_virtual_resource, 2, 0, 0},	
		{"GetDevice", upnp_get_device, 1, 0, 0},	
		{"FindService", upnp_find_service, 1, 0, 0},	
		{"CreateDevice", upnp_create_device, 2, 0, 0},	
		{"DeleteDevice", upnp_delete_device, 1, 0, 0},	
		{0, 0, 0, 0, 0}
	};

	if (param->unload) {
		if (m_nbJSInstances) {
			m_nbJSInstances--;
			if (m_pJSCtx==param->ctx) m_nbJSInstances = 0;

			if (!m_nbJSInstances) {
				if (m_pGenericController) {
					u32 i, count;
					count = gf_list_count(m_pGenericController->m_Devices);
					for (i=0; i<count; i++) {
						GPAC_DeviceItem *item = (GPAC_DeviceItem *)gf_list_get(m_pGenericController->m_Devices, i);
						item->DetachJS();
						item->js_ctx = NULL;
					}
				}
				if (m_Devices) {
					while (gf_list_count(m_Devices)) {
						GPAC_GenericDevice *device = (GPAC_GenericDevice*)gf_list_get(m_Devices, 0);
						gf_list_rem(m_Devices, 0);
						device->DetachJS(m_pJSCtx);
					}
					gf_list_del(m_Devices);
					m_Devices = NULL;
				}
				m_pJSCtx = NULL;
			}
		}
		return 0;
	}
	if (m_nbJSInstances) {
		m_nbJSInstances++;
		JS_DefineProperty((JSContext*)param->ctx, (JSObject*)param->global, "UPnP", OBJECT_TO_JSVAL(m_pObj), 0, 0, 0);
		return 0;
	}

	m_pJSCtx = (JSContext*)param->ctx;
	/*setup JS bindings*/
	_SETUP_CLASS(upnpClass, "UPNPMANAGER", JSCLASS_HAS_PRIVATE, upnp_getProperty, JS_PropertyStub, JS_FinalizeStub);

	JS_InitClass(m_pJSCtx, (JSObject*)param->global, 0, &upnpClass, 0, 0, upnpClassProps, upnpClassFuncs, 0, 0);
	m_pObj = JS_DefineObject(m_pJSCtx, (JSObject*)param->global, "UPnP", &upnpClass, 0, 0);
	JS_SetPrivate(m_pJSCtx, m_pObj, this);

	_SETUP_CLASS(upnpDeviceClass, "UPNPAVDEVICE", JSCLASS_HAS_PRIVATE, JS_PropertyStub, JS_PropertyStub, JS_FinalizeStub);
	
	/*setup JS bindings*/
	JSPropertySpec upnpDeviceClassProps[] = {
		{0, 0, 0, 0, 0}
	};
	JSFunctionSpec upnpDeviceClassFuncs[] = {
		{"FindService", upnp_device_find_service, 0, 0, 0},
		{0, 0, 0, 0, 0}
	};
	_SETUP_CLASS(upnpGenericDeviceClass, "UPNPDEVICE", JSCLASS_HAS_PRIVATE, upnpdevice_getProperty, JS_PropertyStub, JS_FinalizeStub);
	JS_InitClass(m_pJSCtx, (JSObject*)param->global, 0, &upnpGenericDeviceClass, 0, 0, upnpDeviceClassProps, upnpDeviceClassFuncs, 0, 0);
	
	_SETUP_CLASS(upnpServiceClass, "UPNPSERVICEDEVICE", JSCLASS_HAS_PRIVATE, upnpservice_getProperty, JS_PropertyStub, JS_FinalizeStub);
	JS_InitClass(m_pJSCtx, (JSObject*)param->global, 0, &upnpServiceClass, 0, 0, 0, 0, 0, 0);


	m_nbJSInstances=1;
	
	upnp_init_time = gf_sys_clock();

	count = gf_cfg_get_key_count(m_pTerm->user->config, "UPnPDevices");
	for (i=0; i<count; i++) {
		char szFriendlyName[1024], szFile[1024], *sep;
		const char *device_id = gf_cfg_get_key_name(m_pTerm->user->config, "UPnPDevices", i);
		const char *dev = gf_cfg_get_key(m_pTerm->user->config, "UPnPDevices", device_id);

		if (!strncmp(dev, "off;", 4)) continue;
		
		if (!strncmp(dev, "on;", 3)) dev += 3;

		sep = (char*)strchr(dev, ';');
		if (!sep) continue;
		sep[0] = 0;
		strcpy(szFile, dev);
		sep[0] = ';';
		
		if (!sep[1]) continue;
		strcpy(szFriendlyName, sep+1);

		FILE *f = fopen(szFile, "rt");
		if (!f) continue;


		GPAC_GenericDevice *device = upnp_create_generic_device(this, (JSObject*)param->global, device_id, szFriendlyName);
		device->js_source = szFile;

		jsval aval;
		fseek(f, 0, SEEK_END);
		u32 size = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buf = (char*)gf_malloc(sizeof(char)*(size+1));
		size = fread(buf, 1, size, f);
		buf[size]=0;
		/*evaluate the script on the object only*/
		if (JS_EvaluateScript(m_pJSCtx, device->obj, buf, size, 0, 0, &aval) != JS_TRUE) {
			JS_RemoveRoot(m_pJSCtx, &device->obj);
			GF_LOG(GF_LOG_ERROR, GF_LOG_NETWORK, ("[UPnP] Unable to load device %s: script error in %s\n", szFriendlyName, szFile));
			gf_list_del_item(m_Devices, device);
			delete device;
		}
		fclose(f);
		gf_free(buf);
	}
	return 1;
}

#endif

static Bool upnp_process(GF_TermExt *termext, u32 action, void *param)
{
	const char *opt;
	GF_UPnP *upnp = (GF_UPnP *) termext->udta;

	switch (action) {
	case GF_TERM_EXT_START:
		opt = gf_modules_get_option((GF_BaseInterface*)termext, "UPnP", "Enabled");
		if (!opt) {
			opt = "yes";
			gf_modules_set_option((GF_BaseInterface*)termext, "UPnP", "Enabled", "yes");
		}
		if (!strcmp(opt, "yes")) {
			upnp->Load((GF_Terminal *)param);
			return 1;
		}
		return 0;

	case GF_TERM_EXT_STOP:
		upnp->Unload();
		break;

	case GF_TERM_EXT_PROCESS:
#ifdef GPAC_HAS_SPIDERMONKEY
		if (upnp->m_Devices) {
			u32 now;
			now = gf_sys_clock() - upnp->upnp_init_time;
			if (now - upnp->last_time > 200) {
				u32 i, count; 
				jsval argv[1], rval;
				upnp->LockTerminal(1);
				argv[0] = DOUBLE_TO_JSVAL( JS_NewDouble(upnp->m_pJSCtx, (Double)now / 1000.0) );
				count = gf_list_count(upnp->m_Devices);
				for (i=0; i<count; i++) {
					GPAC_GenericDevice *device = (GPAC_GenericDevice *)gf_list_get(upnp->m_Devices, i);
					if (device->run_proc)
						JS_CallFunctionValue(upnp->m_pJSCtx, device->obj, device->run_proc, 1, argv, &rval);
				}
				upnp->LockTerminal(0);
				upnp->last_time = now;
			}
		}
#endif
		break;

	case GF_TERM_EXT_EVENT:
		return upnp->ProcessEvent((GF_Event*)param);
#ifdef GPAC_HAS_SPIDERMONKEY
	case GF_TERM_EXT_JSBIND:
		return upnp->LoadJS((GF_TermExtJS*)param);
#endif
	}
	return 0;
}


GF_TermExt *upnp_new()
{
	GF_TermExt *dr;
	GF_UPnP *ext;
	dr = (GF_TermExt *) gf_malloc(sizeof(GF_TermExt));
	memset(dr, 0, sizeof(GF_TermExt));
	GF_REGISTER_MODULE_INTERFACE(dr, GF_TERM_EXT_INTERFACE, "GPAC UPnP Platinum", "gpac distribution");

	dr->process = upnp_process;
	ext = new GF_UPnP();
	dr->udta = ext;
	ext->term_ext = dr;

	dr->caps = GF_TERM_EXTENSION_FILTER_EVENT | GF_TERM_EXTENSION_NOT_THREADED;
#ifdef GPAC_HAS_SPIDERMONKEY
	dr->caps |= GF_TERM_EXTENSION_JS;
#endif
	return dr;
}


void upnp_delete(GF_BaseInterface *ifce)
{
	GF_TermExt *dr = (GF_TermExt *) ifce;
	GF_UPnP *ext = (GF_UPnP *) dr->udta;
	delete ext;
	gf_free(dr);
}



#ifdef __cplusplus
extern "C" {
#endif

GF_EXPORT
const u32 *QueryInterfaces() 
{
	static u32 si [] = {
		GF_TERM_EXT_INTERFACE,
		0
	};
	return si; 
}

GF_EXPORT
GF_BaseInterface *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == GF_TERM_EXT_INTERFACE) return (GF_BaseInterface *)upnp_new();
	return NULL;
}

GF_EXPORT
void ShutdownInterface(GF_BaseInterface *ifce)
{
	switch (ifce->InterfaceType) {
	case GF_TERM_EXT_INTERFACE:
		upnp_delete(ifce);
		break;
	}
}

#ifdef __cplusplus
}
#endif