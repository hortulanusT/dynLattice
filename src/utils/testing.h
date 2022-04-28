/**
 * @file testing.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief several compiler macros for faster testing of variables
 * @version 0.1
 * @date 2021-10-25
 * 
 * @copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */
#pragma once
#include <jem/base/System.h>
#include <jem/base/Exception.h>
#include <jem/base/RuntimeException.h>
#include <jive/util/Printer.h>
#include <iostream>

#ifndef NDEBUG
  #define REPORT( key ) jem::System::debug( myName_ ) << "\n-=-=-=-=-=-=-=-=-=-=- " << #key << " :: " << key << " -=-=-=-=-=-=-=-=-=-=-\n";
  #define SUBHEADER2( a, b ) jem::System::debug( myName_ ) << "-=-=-=-=- " << #a << " :: " << a << " -=-=-=-=- " << #b << " :: " << b << " -=-=-=-=-\n";
  #define TEST_CONTEXT( key ) jem::System::debug( myName_ ) << "> > > " << #key << " in \"" << getContext() << "\" function \"" << __FUNCTION__ <<  "\" ::\n" << key << "\n";
  #define TEST_NO_CONTEXT( key ) jem::System::debug( __FUNCTION__ ) << "> > > " << #key << " in \"" << __PRETTY_FUNCTION__ << "\" ::\n" << key << "\n";
  #define TEST_PRINTER( key ) key->printTo( jive::util::Printer::get() ); jive::util::Printer::flush();
  #define TEST_STD( key ) std::clog << "> > > " << #key << " ::\n" << key << "\n";
  #define WARN_ASSERT2( expr, msg ) if ( ! expr ) jem::System::warn() << msg << "\n";
  #define STOP_DEBUG throw jem::RuntimeException(__FUNCTION__, "I want to end here :)");
#else
  #define REPORT( key )
  #define SUBHEADER2( a, b )
  #define TEST_CONTEXT( key )
  #define TEST_NO_CONTEXT( key )
  #define TEST_PRINTER ( key )
  #define TEST_STD( key )
  #define WARN_ASSERT2( expr, msg )
  #define STOP_DEBUG
#endif

#define NOT_IMPLEMENTED throw jem::RuntimeException( __PRETTY_FUNCTION__, "Function not implemented!" );