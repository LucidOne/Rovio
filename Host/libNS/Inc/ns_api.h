//----------------------------------------------------------------------------
//  Copyright (C) 2001-2008, Evolution Robotics, Inc.
//  This material contains trade secrets and confidential information
//  of Evolution Robotics, Inc.  Any use, reproduction, disclosure or
//  dissemination is strictly prohibited without the explicit written
//  permission of Evolution Robotics, Inc.  All rights reserved.
//----------------------------------------------------------------------------
//                                          NorthStar based navigation module
//----------------------------------------------------------------------------
#ifndef NS_API_H
#define NS_API_H

// Lower number means higher priority
#define NS_THREAD_PRIORITY   10

void er_init_ns_nav_priority( int pr );

#define er_init_ns_nav() er_init_ns_nav_priority(NS_THREAD_PRIORITY)

void er_register_ns_cgi();

// Battery status
void ERSP_get_battery_status(unsigned char* battery_charge_value,
                             unsigned char* battery_charging_status);

// resume/suspend thread
bool ERSP_rovio_libns_thread_suspend_allowed();
void ERSP_rovio_libns_thread_resume();
void ERSP_rovio_libns_thread_suspend();
int  ERSP_rovio_libns_restore_defaults();

#endif
//----------------------------------------------------------------------------
