/**************************************************************************
 * Copyright(c) 2005-2007, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

//-----------------------------------------------------------------------//
// ITS PID class --- method # 2                                          //
//                                                                       //
//                                                                       //
//The PID is based on the likelihood of all the four ITS' layers,        //
//without using the truncated mean for the dE/dx. The response           //
//functions for each layer are convoluted Landau-Gaussian functions.     // 
// Origin: Elena Bruna bruna@to.infn.it, Massimo Masera masera@to.infn.it//
//-----------------------------------------------------------------------//

#include "AliITSpidESD2.h"
#include "AliESDEvent.h"
#include "AliESDtrack.h"
#include "AliITSLoader.h"
#include "AliITSPidParams.h"
#include "AliITSPident.h"
#include "AliLog.h"

ClassImp(AliITSpidESD2)
//_________________________________________________________________________
AliITSpidESD2::AliITSpidESD2():
  AliITSpidESD(),
  fPidPars(0)
{ 
  //  The main constructor
  fPidPars=new AliITSPidParams("default");
}


//_________________________________________________________________________
AliITSpidESD2::~AliITSpidESD2(){
  //destructor

  if(fPidPars)delete fPidPars;
}


//______________________________________________________________________
AliITSpidESD2::AliITSpidESD2(const AliITSpidESD2 &ob) :AliITSpidESD(ob),
fPidPars(ob.fPidPars) 
{
  // Copy constructor
}


//______________________________________________________________________
AliITSpidESD2& AliITSpidESD2::operator=(const AliITSpidESD2& ob ){
  // Assignment operator
  this->~AliITSpidESD2();
  new(this) AliITSpidESD2(ob);
  return *this;
}

    
//______________________________________________________________________
void AliITSpidESD2::GetITSpidSingleTrack(AliESDtrack* esdtr, Double_t condprobfun[]){
  // Method to calculate PID probabilities for a single track
  
  
  Double_t prip=0.33;
  Double_t prik=0.33;
  Double_t pripi=0.33;
  Double_t prie=0.;
  AliITSPident mypid(esdtr,fPidPars,prip,prik,pripi,prie);
  condprobfun[0]=mypid.GetProdCondFunPi();//el --PID in the ITS does not distinguish among Pi,el,mu
  condprobfun[1]=mypid.GetProdCondFunPi();//mu
  condprobfun[2]=mypid.GetProdCondFunPi();//pi
  condprobfun[3]=mypid.GetProdCondFunK();//kaon
  condprobfun[4]=mypid.GetProdCondFunPro();//pro
  return;
}



//_________________________________________________________________________
Int_t AliITSpidESD2::MakePID(AliESDEvent *event){
  //  This function calculates the "detector response" PID probabilities 

  Int_t ntrk=event->GetNumberOfTracks();
  const Int_t kns=AliPID::kSPECIES;
  Double_t condprobfun[kns];
    
  for (Int_t i=0; i<ntrk; i++) {
    AliESDtrack *esdtr=event->GetTrack(i);
    if ((esdtr->GetStatus()&AliESDtrack::kITSin )==0){
      if ((esdtr->GetStatus()&AliESDtrack::kITSout)==0) continue;
    }
    GetITSpidSingleTrack(esdtr,condprobfun);
    esdtr->SetITSpid(condprobfun);
  }
  return 0;
}
