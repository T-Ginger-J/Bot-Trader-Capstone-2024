﻿/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once
#ifndef TEST_CPP_CLIENT_STDAFX_H
#define TEST_CPP_CLIENT_STDAFX_H
#define _CRT_SECURE_NO_WARNINGS

#define UNICODE
#define _UNICODE

#include "client/StdAfx.h"
#include <thread>
#include <iostream>
#include <fstream>

#include <windows.h>
#include <direct.h>
#include <Shlwapi.h>
#include <commdlg.h>

#ifndef TWSAPIDLL
#ifndef TWSAPIDLLEXP
#ifdef _MSC_VER
#define TWSAPIDLLEXP __declspec(dllimport)
#else
#define TWSAPIDLLEXP
#endif
#endif
#endif
#endif