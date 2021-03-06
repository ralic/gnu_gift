// -*- mode: c++ -*- 
/* 

    GIFT, a flexible content based image retrieval system.
    Copyright (C) 1998, 1999, 2000, 2001, 2002, CUI University of Geneva

     Copyright (C) 2003, 2004 Bayreuth University
    Copyright (C) 2005,2006 Bamberg University
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#include "libMRML/include/CDynamicQueryFactory.h"
#include "libMRML/include/CQMultiple.h"
//#warning hard perl switchoff this should not go to distro
#include "libGIFTQuPerl/include/CQPerl.h"
#include "libMRML/include/GIFTExceptions.h"
#include "libMRML/include/my_throw.h"

// for file name treatment
#include "libMRML/include/getLibNameFromFileName.h"
#include <set>
#include <cstdlib>

CQuery* CDynamicQueryFactory::makeQuery(const string & inBaseType, 
					   CAccessorAdminCollection & inAccessorAdminCollection,
					   CAlgorithm & inAlgorithm){
  if(inBaseType=="multiple"){
    return(new CQMultiple(inAccessorAdminCollection,
			  inAlgorithm));
  }
  
#ifdef __GIFT_WITH_PERL__
  if(inBaseType=="perl"){
    return(new CQPerl(inAccessorAdminCollection,
 		      inAlgorithm));
  }
#endif

  const_iterator lFound(find(inBaseType));
  if(lFound!=end()){
    cout << "CDynamicQueryFactory" << endl;
    return lFound->second->makeQuery(inAccessorAdminCollection,
				     inAlgorithm);
  }else{
    cerr << "CDynamicQueryFactory: " << inBaseType << " was not found";
    my_throw(VEConfigurationError(inBaseType.c_str()));
  }
}

CDynamicQueryFactory::CDynamicQueryFactory(string inDirectoryName){
  cout << "--" << endl
       << "Configuring CDynamicQueryFactory " << endl
       << "Looking for libGIFTQu*.so in directory " << endl
       << inDirectoryName << endl; 
  set<string> lSeenLibs;
  DIR* lDirectory(opendir(inDirectoryName.c_str()));  
  if(!lDirectory){
    // this means the library has disappeared.
    cerr << "FATAL ERROR, cannot find " << inDirectoryName << endl;
    
    exit(20);
  }else{
    dirent* lDirectoryEntry;
    while(lDirectoryEntry=readdir(lDirectory)){
      string lFileName(lDirectoryEntry->d_name);

      pair<bool,string> lIsLibAndLibName(getLibNameFromFileName("libGIFTQu",lFileName));
      bool lIsLib(lIsLibAndLibName.first);
      string lLibName(lIsLibAndLibName.second);

      if(lIsLib && (lSeenLibs.find(lLibName)==lSeenLibs.end())){
	CQueryPlugin* lPlugin(new CQueryPlugin(inDirectoryName,
					       lFileName,
					       lLibName));
	if(lPlugin->isSane()){
	  cout << lFileName << " contains a sane GIFT Query plugin: " << lPlugin->getName() << endl;
	  insert(make_pair(string(lPlugin->getName()),
			   lPlugin));
	  lSeenLibs.insert(lLibName);
	}else{
	  delete lPlugin;
	}
      }else{
	if(!lIsLib){
	  cout << "Not testing file:" << lFileName << " (File name does not match plugin name) " << endl;
	}else{
	  cout << "Lib:" << lLibName << ", to be linked from " << lFileName << " already registered! " << endl;
	}
      }
    }
  }
  cout << "FINISHED Configuring CDynamicQueryFactory " << endl;
}

