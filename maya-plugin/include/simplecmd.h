/**
* @file simplecmd.h
* @author Erik Sandrén
* @date 29-11-2016
* @brief [Description Goes Here]
*/

#ifndef __SIMPLECMD_H__
#define __SIMPLECMD_H__

#include <maya/MPxCommand.h>

class SimpleCmd : public MPxCommand {
public:
  static void* creator() { return new SimpleCmd(); }
  MStatus doIt(const MArgList& args) { return MStatus::kSuccess; }
};



#endif
