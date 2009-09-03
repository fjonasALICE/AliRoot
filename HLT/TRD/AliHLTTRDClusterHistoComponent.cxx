//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors: Sylwester Radomski radomski@physi.uni-heidelberg.de    *
//*                  for The ALICE HLT Project.                            *
//*                                                                        *
//* Permission to use, copy, modify and distribute this software and its   *
//* documentation strictly for non-commercial purposes is hereby granted   *
//* without fee, provided that the above copyright notice appears in all   *
//* copies and that both the copyright notice and this permission notice   *
//* appear in the supporting documentation. The authors make no claims     *
//* about the suitability of this software for any purpose. It is          *
//* provided "as is" without express or implied warranty.                  *
//**************************************************************************

/** @file   AliHLTTRDClusterHistoComponent.cxx
    @author Sylwester Radomski
    @brief  Component for ploting charge in clusters
*/

#if __GNUC__>= 3
using namespace std;
#endif

#include "AliHLTTRDClusterHistoComponent.h"
#include "AliHLTTRDDefinitions.h"
#include "AliHLTTRDCluster.h"
#include "AliTRDcluster.h"
#include "AliCDBEntry.h"
#include "AliCDBManager.h"
#include <TFile.h>
#include <TString.h>
#include "TObjString.h"
#include "TClonesArray.h"
#include "AliHLTTRDUtils.h"

//#include "AliHLTTRD.h"
//#include <stdlib.h>
//#include <cerrno>

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTRDClusterHistoComponent)

AliHLTTRDClusterHistoComponent::AliHLTTRDClusterHistoComponent()
: fClusterArray(NULL),
  fNClsDet(NULL),
  fClsAmp(NULL),
  fClsAmpDrift(NULL),
  fClsTB(NULL),
  fClsAmpDist(NULL),
  fSClsDist(NULL)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt

}

AliHLTTRDClusterHistoComponent::~AliHLTTRDClusterHistoComponent()
{
  // see header file for class documentation
}

// Public functions to implement AliHLTComponent's interface.
// These functions are required for the registration process

const char* AliHLTTRDClusterHistoComponent::GetComponentID()
{
  // see header file for class documentation
  
  return "TRDClusterHisto";
}

void AliHLTTRDClusterHistoComponent::GetInputDataTypes(AliHLTComponentDataTypeList& list)
{
  // see header file for class documentation
  list.clear();
  list.push_back( AliHLTTRDDefinitions::fgkClusterDataType );
}

AliHLTComponentDataType AliHLTTRDClusterHistoComponent::GetOutputDataType()
{
  // see header file for class documentation
  return kAliHLTDataTypeHistogram  | kAliHLTDataOriginTRD;

}

void AliHLTTRDClusterHistoComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
{
  // see header file for class documentation
  constBase = 5000;
  inputMultiplier = 3;
}

AliHLTComponent* AliHLTTRDClusterHistoComponent::Spawn()
{
  // see header file for class documentation
  return new AliHLTTRDClusterHistoComponent;
}

int AliHLTTRDClusterHistoComponent::DoInit(int /*argc*/, const char** /*argv*/ )
{
  // Initialize histograms

  fClusterArray = new TClonesArray("AliTRDcluster");

  fNClsDet = new TH1D("trdClsDet", ";detector", 540, -0.5, 539.5);
  fClsAmp  = new TH1D("trdClsAmp", ";amplitude", 200, -0.5, 1999.5);
  fClsAmpDrift = new TH1D("trdClsAmpDrift", ";amplitude", 200, -0.5, 199.5) ;
  fClsTB = new TH1D("trdClsTB", ";time bin", 35, -0.5, 34.5);
  fClsAmpDist = new TH1D("trdClsAmpDist", "mean amplitude", 200, 0, 1000);
  fSClsDist = new TH1D("sclsdist", "Super cluster spectrum", 200, 0, 8000);

  for(int i=0; i<540; i++)
    fClsAmpDriftDet[i] = new TH1D(Form("trdClsDriftDet_%d",i), "", 200, -0.5, 199.5);
    
    return 0;
}
  
int AliHLTTRDClusterHistoComponent::DoDeinit()
{
  // see header file for class documentation

  fClusterArray->Delete();
  delete fClusterArray;

  // delete histograms
  if (fNClsDet) delete fNClsDet;
  if (fClsAmp) delete fClsAmp;
  if (fClsAmpDrift) delete fClsAmpDrift;
  if (fClsTB) delete fClsTB;
  if (fClsAmpDist) delete fClsAmpDist;
  if (fSClsDist) delete fSClsDist;

  for(int i=0; i<540; i++)
    if (fClsAmpDriftDet[i]) delete fClsAmpDriftDet[i];

  return 0;
}

int AliHLTTRDClusterHistoComponent::DoEvent(const AliHLTComponentEventData& /*evtData*/, 
					    AliHLTComponentTriggerData& /*trigData*/)
{

  if (GetFirstInputBlock(kAliHLTDataTypeSOR) || GetFirstInputBlock(kAliHLTDataTypeEOR)) return 0;
  
  const AliHLTComponentBlockData* iter = NULL;
  
  Float_t sClusterCharge[540] = { 0 };

  for ( iter = GetFirstInputBlock(AliHLTTRDDefinitions::fgkClusterDataType); 
	iter != NULL; iter = GetNextInputBlock() ) {

    HLTDebug("We get the right data type: Block Ptr: 0x%x; Block Size: %i",
	     iter->fPtr, iter->fSize);

    AliHLTTRDUtils::ReadClusters(fClusterArray, iter->fPtr, iter->fSize);
    HLTDebug("TClonesArray of clusters: nbEntries = %i", fClusterArray->GetEntriesFast());

    AliTRDcluster *cls;

    // loop over clusters 
    for(int i=0;i<fClusterArray->GetEntriesFast();i++) {

      cls=(AliTRDcluster*)fClusterArray->At(i);
      
      fNClsDet->Fill(cls->GetDetector());
      sClusterCharge[cls->GetDetector()] += cls->GetQ();
      fClsAmp->Fill(cls->GetQ());
      
      int tb = cls->GetPadTime();
      fClsTB->Fill(tb);
      if (tb > 5 && tb <25)
	fClsAmpDrift->Fill(cls->GetQ()); 
      
      fClsAmpDriftDet[cls->GetDetector()]->Fill(cls->GetQ());
    }
    
    fClusterArray->Delete();
    
  }
   
  fClsAmpDist->Reset();
  for(int det=0; det<540; det++) {
    if (fClsAmpDriftDet[det]->GetSum() > 0) 
      fClsAmpDist->Fill(fClsAmpDriftDet[det]->GetMean());
    fSClsDist->Fill(sClusterCharge[det]);
  }

  PushBack((TObject*)fNClsDet, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);   
  PushBack((TObject*)fClsAmp, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);  
  PushBack((TObject*)fClsAmpDrift, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);   
  PushBack((TObject*)fClsTB, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);  
  PushBack((TObject*)fClsAmpDist, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);  
  PushBack((TObject*)fSClsDist, kAliHLTDataTypeHistogram | kAliHLTDataOriginTRD, 0);  
    
  return 0;
}
