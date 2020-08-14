#include "ccontroller.h"
#define _joystickhash ('j' + 's')

#include "libudev.h"

/*
 * TODO: Look at this: https://docs.libretro.com/guides/input-joypad-drivers/
 * Use this to automate setting up raw input
 *https://c-qs.blogspot.com/2012/06/libudev-and-sysfs-tutorial.html

subsystems: input is for kb, ms and gamepads
sound is for rumble in controllers
 * */

_global LibHandle udev_lib = 0;

// MARK: udev functions
_global udev_enumerate* (*udev_enumerate_new_fptr)(udev*) = 0;

_global s32 (*udev_enumerate_add_match_subsystem_fptr)(udev_enumerate*,
						       const s8*) = 0;

_global s32 (*udev_enumerate_scan_devices_fptr)(udev_enumerate*) = 0;

_global udev_list_entry* (*udev_enumerate_get_list_entry_fptr)(
    udev_enumerate*) = 0;

_global udev_list_entry* (*udev_list_entry_get_next_fptr)(udev_list_entry*) = 0;

_global const s8* (*udev_list_entry_get_name_fptr)(udev_list_entry*) = 0;

_global udev_device* (*udev_device_new_from_syspath_fptr)(udev*, const s8*) = 0;

_global udev_device* (*udev_device_unref_fptr)(udev_device*) = 0;

_global udev_enumerate* (*udev_enumerate_unref_fptr)(udev_enumerate*) = 0;

_global const s8* (*udev_device_get_devnode_fptr)(udev_device*) = 0;

_global const s8* (*udev_device_get_subsystem_fptr)(udev_device*) = 0;

_global const s8* (*udev_device_get_property_value_fptr)(udev_device*,
							 const s8*) = 0;

void InternalLoadUDev() {
	auto lib = udev_lib;

	udev_enumerate_new_fptr = (udev_enumerate * (*)(udev*))
	    LGetLibFunction(lib, "udev_enumerate_new");

	udev_enumerate_add_match_subsystem_fptr =
	    (s32(*)(udev_enumerate*, const s8*))LGetLibFunction(
		lib, "udev_enumerate_add_match_subsystem");

	udev_enumerate_scan_devices_fptr =
	    (s32(*)(udev_enumerate*))LGetLibFunction(
		lib, "udev_enumerate_scan_devices");

	udev_enumerate_get_list_entry_fptr =
	    (udev_list_entry * (*)(udev_enumerate*))
		LGetLibFunction(lib, "udev_enumerate_get_list_entry");

	udev_list_entry_get_next_fptr =
	    (udev_list_entry * (*)(udev_list_entry*))
		LGetLibFunction(lib, "udev_list_entry_get_next");

	udev_list_entry_get_name_fptr =
	    (const s8* (*)(udev_list_entry*))LGetLibFunction(
		lib, "udev_list_entry_get_name");

	udev_device_new_from_syspath_fptr =
	    (udev_device * (*)(udev*, const s8*))
		LGetLibFunction(lib, "udev_device_new_from_syspath");

	udev_device_unref_fptr = (udev_device * (*)(udev_device*))
	    LGetLibFunction(lib, "udev_device_unref");

	udev_enumerate_unref_fptr = (udev_enumerate * (*)(udev_enumerate*))
	    LGetLibFunction(lib, "udev_enumerate_unref");

	udev_device_get_devnode_fptr =
	    (const s8* (*)(udev_device*))LGetLibFunction(
		lib, "udev_device_get_devnode");

	udev_device_get_subsystem_fptr =
	    (const s8* (*)(udev_device*))LGetLibFunction(
		lib, "udev_device_get_subsystem");

	udev_device_get_property_value_fptr =
	    (const s8* (*)(udev_device*, const s8*))LGetLibFunction(
		lib, "udev_device_get_property_value");
}

enum UDEVICE_TYPE {
	UDEVICE_TYPE_NONE,
	UDEVICE_TYPE_CONTROLLER,

};

_global const s8* active_controller_paths_array[16] = {};
_global u32 active_controller_paths_count = 0;

enum UDEVICE_CLASS {
	UDEVICE_CLASS_NONE = 0,
	UDEVICE_CLASS_SOUND = (1 << 0),
	UDEVICE_CLASS_JOYSTICK = (1 << 1),
	UDEVICE_CLASS_ACLRMETER = (1 << 2),
	UDEVICE_CLASS_MOUSE = (1 << 3),
	UDEVICE_CLASS_TOUCH = (1 << 4),
	UDEVICE_CLASS_KEY = (1 << 5),
};

void PrintUClass(udev_device* dev,u32 uclass) {
	u32 entry[] = {
	    UDEVICE_CLASS_SOUND,     UDEVICE_CLASS_JOYSTICK,
	    UDEVICE_CLASS_ACLRMETER, UDEVICE_CLASS_MOUSE,
	    UDEVICE_CLASS_TOUCH,     UDEVICE_CLASS_KEY,
	};

	auto path = udev_device_get_devnode_fptr(dev);

	if(!path){
		return;
	}

	printf("%s = ", path);

	for (u32 i = 0; i < _arraycount(entry); i++) {
		auto e = entry[i];

		switch (uclass & e) {
			case UDEVICE_CLASS_SOUND: {
				printf("UDEVICE_CLASS_SOUND | ");
			} break;
			case UDEVICE_CLASS_ACLRMETER: {
				printf("UDEVICE_CLASS_ACLR | ");
			} break;
			case UDEVICE_CLASS_TOUCH: {
				printf("UDEVICE_CLASS_TOUCH | ");
			} break;
			case UDEVICE_CLASS_JOYSTICK: {
				printf("UDEVICE_CLASS_JOYSTICK | ");
			} break;
			case UDEVICE_CLASS_MOUSE: {
				printf("UDEVICE_CLASS_MOUSE | ");
			} break;
			case UDEVICE_CLASS_KEY: {
				printf("UDEVICE_CLASS_KEY | ");
			} break;
		}
	}

	printf("\n");

	//TODO: print the HID details using ioctl
}

u32 InternalFindMainUDevice(udev_device* dev) { 
	//TODO:
	return 0; 
}

UDEVICE_TYPE InternalUDevGetDeviceType(udev_device* dev) {
	auto subsys = udev_device_get_subsystem_fptr(dev);

	u32 uclass = (u32)UDEVICE_CLASS_NONE;

	if (PCmpString(subsys, "sound")) {
		uclass |= UDEVICE_CLASS_SOUND;
	}

	else if (PCmpString(subsys, "input")) {
		/* The undocumented rule is:
			  - All devices with keys get ID_INPUT_KEY
			  - From this subset, if they have ESC, numbers, and Q
		   to D, it also gets ID_INPUT_KEYBOARD

			  Ref:
		   http://cgit.freedesktop.org/systemd/systemd/tree/src/udev/udev-builtin-input_id.c#n183
		       */

		struct Entry {
			const s8* string;
			UDEVICE_CLASS uclass;
		};

		Entry entry[] = {
		    {"ID_INPUT_JOYSTICK", UDEVICE_CLASS_JOYSTICK},
		    {"ID_INPUT_ACCELEROMETER", UDEVICE_CLASS_ACLRMETER},
		    {"ID_INPUT_MOUSE", UDEVICE_CLASS_MOUSE},
		    {"ID_INPUT_TOUCHSCREEN", UDEVICE_CLASS_TOUCH},
		    {"ID_INPUT_KEY", UDEVICE_CLASS_KEY},
		};

		for (u32 i = 0; i < _arraycount(entry); i++) {
			auto e = entry[i];

			auto val =
			    udev_device_get_property_value_fptr(dev, e.string);

			if (!val) {
				continue;
			}

			if (PCmpString(val, "1")) {
				uclass |= (u32)e.uclass;
			}
		}
	}

	if (!uclass) {
		// this is the old input class style
		auto val = udev_device_get_property_value_fptr(dev, "ID_CLASS");

		if (!val) {
			goto do_default;
		}

		switch (PHashString(val)) {
			case PHashString("joystick"): {
				uclass |= UDEVICE_CLASS_JOYSTICK;
			} break;
			case PHashString("mouse"): {
				uclass |= UDEVICE_CLASS_MOUSE;
			} break;

			case PHashString("kbd"): {
				uclass |= UDEVICE_CLASS_KEY;
			} break;

			default: {
			do_default:
				uclass |= InternalFindMainUDevice(dev);
			} break;
		}
	}

	PrintUClass(dev,uclass);

	return UDEVICE_TYPE_NONE;
}

void InternalUDevPostDevice(udev_device* dev) {
	auto type = InternalUDevGetDeviceType(dev);

	if (type == UDEVICE_TYPE_CONTROLLER) {
		auto path = udev_device_get_devnode_fptr(dev);

		for (u32 i = 0; i < active_controller_paths_count; i++) {
			auto s = active_controller_paths_array[i];

			if (PHashString(path) == PHashString(s)) {
				return;
			}
		}

		auto n = (s8*)alloc(strlen(path) + 1);

		memcpy(n, path, strlen(path) + 1);

		active_controller_paths_array[active_controller_paths_count] =
		    n;
		active_controller_paths_count++;
	}
}

void InternalUDeviceRemoveDevice(udev_device* dev) {}

b32 InternalUDevScan(udev* d) {
	auto en = udev_enumerate_new_fptr(d);

	if (!en) {
		return false;
	}

	udev_enumerate_add_match_subsystem_fptr(en, "input");
	udev_enumerate_add_match_subsystem_fptr(en, "sound");

	udev_enumerate_scan_devices_fptr(en);

	auto l = udev_enumerate_get_list_entry_fptr(en);

	for (auto i = l; i != 0; i = udev_list_entry_get_next_fptr(i)) {
		auto path = udev_list_entry_get_name_fptr(i);
		auto dev = udev_device_new_from_syspath_fptr(d, path);

#if 0
		printf("%s : ",path);
#endif

		if (dev) {
			{
				InternalUDevPostDevice(dev);
			}
			udev_device_unref_fptr(dev);
		}
	}

	udev_enumerate_unref_fptr(en);

	return true;
}

b32 InternalInitUDev() {
	udev_lib = LLoadLibrary("libudev.so");

	InternalLoadUDev();

	auto lib = udev_lib;

	_LOADFOT(udev*, udev_unref, udev*);
	_LOADFOT(udev*, udev_new, void);
	_LOADFOT(udev_monitor*, udev_monitor_new_from_netlink, udev*,
		 const s8*);

	_LOADFOT(s32, udev_monitor_filter_add_match_subsystem_devtype, udev*,
		 const s8*, const s8*);

	_LOADFOT(s32, udev_monitor_enable_receiving, udev_monitor*);

	auto d = udev_new_fptr();

	if (!d) {
		return false;
	}

	auto mon = udev_monitor_new_from_netlink_fptr(d, "udev");

	if (!mon) {
		return false;
	}

	udev_monitor_filter_add_match_subsystem_devtype_fptr(mon, "input", 0);

	udev_monitor_filter_add_match_subsystem_devtype_fptr(mon, "sound", 0);

	udev_monitor_enable_receiving_fptr(mon);

	// MARK: don't do this here. we need this to persist
	// udev_unref_fptr(d);

	InternalUDevScan(d);

	exit(0);

	_breakpoint();

	return true;
}

#ifdef __cplusplus
extern "C" {
#endif

b32 CInitControllers() {
	b32 res = InternalInitUDev();

	return res;
}

#ifdef __cplusplus
}
#endif

#if 0

//hash the string instead. it'll be faster
u32 ParseIndex(const s8* string){
  return 0;
}

Controller CreateController(const s8* joystickpath){
  return {FOpenFile(joystickpath,F_FLAG_READONLY),ParseIndex(&(joystickpath[2]))};
}

void CRefreshControllers(ControllerList* controller_list){

  DirectoryHandle dirhandle;
  FileInfo info;
  
  u32 res = FFindFirstFile("/dev/input/",&dirhandle,&info);

  if(!res){
    return;
  }

  goto processname;

  //if info.filename contains js and index is not in the list, add

  while(FFindNextFile(&dirhandle,&info)){

  processname:
    //check if 'js'
    u32 hash = info.filename[0] + info.filename[1];

    if(hash == _joystickhash){
      s8* index_string = &(info.filename[2]);
      //parse index
      u32 index = ParseIndex(index_string);

      logic hasmatch = 0;
      
      //check if controller index is in list
      for(ptrsize i = 0; i < controller_list-> count; i++){

	if((*controller_list)[i].id == index){
	  hasmatch = 1;
	  break;
	}
	
      }

      if(!hasmatch){
	controller_list->PushBack(CreateController(info.filename));
      }
      
    }
    
  }
  
}

#endif
