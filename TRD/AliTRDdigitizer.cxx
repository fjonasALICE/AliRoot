/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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

/*
$Log$
Revision 1.8  2000/06/09 11:10:07  cblume
Compiler warnings and coding conventions, next round

Revision 1.7  2000/06/08 18:32:58  cblume
Make code compliant to coding conventions

Revision 1.6  2000/06/07 16:27:32  cblume
Try to remove compiler warnings on Sun and HP

Revision 1.5  2000/05/09 16:38:57  cblume
Removed PadResponse(). Merge problem

Revision 1.4  2000/05/08 15:53:45  cblume
Resolved merge conflict

Revision 1.3  2000/04/28 14:49:27  cblume
Only one declaration of iDict in MakeDigits()

Revision 1.1.4.1  2000/05/08 14:42:04  cblume
Introduced AliTRDdigitsManager

Revision 1.1  2000/02/28 19:00:13  cblume
Add new TRD classes

*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Creates and handles digits from TRD hits                                 //
//                                                                           //
//  The following effects are included:                                      //
//      - Diffusion                                                          //
//      - ExB effects                                                        //
//      - Gas gain including fluctuations                                    //
//      - Pad-response (simple Gaussian approximation)                       //
//      - Electronics noise                                                  //
//      - Electronics gain                                                   //
//      - Digitization                                                       //
//      - ADC threshold                                                      //
//  The corresponding parameter can be adjusted via the various              //
//  Set-functions. If these parameters are not explicitly set, default       //
//  values are used (see Init-function).                                     //
//  To produce digits from a root-file with TRD-hits use the                 //
//  slowDigitsCreate.C macro.                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TMath.h>
#include <TVector.h>
#include <TRandom.h>
#include <TROOT.h>
#include <TTree.h>

#include "AliTRD.h"
#include "AliTRDdigitizer.h"
#include "AliTRDdataArrayI.h"
#include "AliTRDdataArrayF.h"
#include "AliTRDdigitsManager.h"

ClassImp(AliTRDdigitizer)

//_____________________________________________________________________________
AliTRDdigitizer::AliTRDdigitizer():TNamed()
{
  //
  // AliTRDdigitizer default constructor
  //

  fInputFile     = NULL;
  fDigits        = NULL;
  fTRD           = NULL;
  fGeo           = NULL;
  fPRF           = NULL;

  fEvent         = 0;
  fGasGain       = 0.0;
  fNoise         = 0.0;
  fChipGain      = 0.0;
  fADCoutRange   = 0.0;
  fADCinRange    = 0.0;
  fADCthreshold  = 0;
  fDiffusionOn   = 0;
  fDiffusionT    = 0.0;
  fDiffusionL    = 0.0;
  fElAttachOn    = 0;
  fElAttachProp  = 0.0;
  fExBOn         = 0;
  fLorentzAngle  = 0.0;

}

//_____________________________________________________________________________
AliTRDdigitizer::AliTRDdigitizer(const Text_t *name, const Text_t *title)
                :TNamed(name,title)
{
  //
  // AliTRDdigitizer default constructor
  //

  fInputFile     = NULL;
  fDigits        = NULL;
  fTRD           = NULL;
  fGeo           = NULL;

  fEvent         = 0;

  Init();

}

//_____________________________________________________________________________
AliTRDdigitizer::AliTRDdigitizer(const AliTRDdigitizer &d)
{
  //
  // AliTRDdigitizer copy constructor
  //

  ((AliTRDdigitizer &) d).Copy(*this);

}

//_____________________________________________________________________________
AliTRDdigitizer::~AliTRDdigitizer()
{
  //
  // AliTRDdigitizer destructor
  //

  if (fInputFile) {
    fInputFile->Close();
    delete fInputFile;
  }

  if (fDigits) {
    delete fDigits;
  }

  if (fPRF) delete fPRF;

}

//_____________________________________________________________________________
AliTRDdigitizer &AliTRDdigitizer::operator=(const AliTRDdigitizer &d)
{
  //
  // Assignment operator
  //

  if (this != &d) ((AliTRDdigitizer &) d).Copy(*this);
  return *this;

}

//_____________________________________________________________________________
void AliTRDdigitizer::Copy(TObject &d)
{
  //
  // Copy function
  //

  ((AliTRDdigitizer &) d).fInputFile     = NULL;
  ((AliTRDdigitizer &) d).fDigits        = NULL;
  ((AliTRDdigitizer &) d).fTRD           = NULL;
  ((AliTRDdigitizer &) d).fGeo           = NULL;

  ((AliTRDdigitizer &) d).fEvent         = 0;

  ((AliTRDdigitizer &) d).fGasGain       = fGasGain;
  ((AliTRDdigitizer &) d).fNoise         = fNoise;
  ((AliTRDdigitizer &) d).fChipGain      = fChipGain;
  ((AliTRDdigitizer &) d).fADCoutRange   = fADCoutRange;
  ((AliTRDdigitizer &) d).fADCinRange    = fADCinRange;
  ((AliTRDdigitizer &) d).fADCthreshold  = fADCthreshold;
  ((AliTRDdigitizer &) d).fDiffusionOn   = fDiffusionOn; 
  ((AliTRDdigitizer &) d).fDiffusionT    = fDiffusionT;
  ((AliTRDdigitizer &) d).fDiffusionL    = fDiffusionL;
  ((AliTRDdigitizer &) d).fElAttachOn    = fElAttachOn;
  ((AliTRDdigitizer &) d).fElAttachProp  = fElAttachProp;
  ((AliTRDdigitizer &) d).fExBOn         = fExBOn;
  ((AliTRDdigitizer &) d).fLorentzAngle  = fLorentzAngle;
  ((AliTRDdigitizer &) d).fLorentzFactor = fLorentzFactor;

  fPRF->Copy(*((AliTRDdigitizer &) d).fPRF);

}

//_____________________________________________________________________________
Int_t AliTRDdigitizer::Diffusion(Float_t driftlength, Float_t *xyz)
{
  //
  // Applies the diffusion smearing to the position of a single electron
  //

  Float_t driftSqrt = TMath::Sqrt(driftlength);
  Float_t sigmaT = driftSqrt * fDiffusionT;
  Float_t sigmaL = driftSqrt * fDiffusionL;
  xyz[0] = gRandom->Gaus(xyz[0], sigmaL * fLorentzFactor);
  xyz[1] = gRandom->Gaus(xyz[1], sigmaT * fLorentzFactor);
  xyz[2] = gRandom->Gaus(xyz[2], sigmaT);
  return 1;

}

//_____________________________________________________________________________
Int_t AliTRDdigitizer::ExB(Float_t driftlength, Float_t *xyz)
{
  //
  // Applies E x B effects to the position of a single electron
  //

  xyz[0] = xyz[0];
  xyz[1] = xyz[1] + fLorentzAngle * driftlength;
  xyz[2] = xyz[2];

  return 1;

}

//_____________________________________________________________________________
void AliTRDdigitizer::Init()
{
  //
  // Initializes the digitization procedure with standard values
  //

  // The default parameter for the digitization
  fGasGain       = 2.0E3;
  fNoise         = 3000.;
  fChipGain      = 10.;
  fADCoutRange   = 255.;
  fADCinRange    = 2000.;
  fADCthreshold  = 1;

  // Transverse and longitudinal diffusion coefficients (Xe/Isobutane)
  fDiffusionOn   = 1;
  fDiffusionT    = 0.060;
  fDiffusionL    = 0.017;

  // Propability for electron attachment
  fElAttachOn    = 0;
  fElAttachProp  = 0.0;

  // E x B effects
  fExBOn         = 0;
  // omega * tau. (tau ~ 12 * 10^-12, B = 0.2T)
  fLorentzAngle  = 17.6 * 12.0 * 0.2 * 0.01;

  // The pad response function
  fPRF           = new TF1("PRF","[0]*([1]+exp(-x*x/(2.0*[2])))",-2,2);
  fPRF->SetParameter(0, 0.8872);
  fPRF->SetParameter(1,-0.00573);
  fPRF->SetParameter(2, 0.454 * 0.454);

}

//_____________________________________________________________________________
Bool_t AliTRDdigitizer::Open(const Char_t *name, Int_t nEvent)
{
  //
  // Opens a ROOT-file with TRD-hits and reads in the hit-tree
  //

  // Connect the AliRoot file containing Geometry, Kine, and Hits
  fInputFile = (TFile*) gROOT->GetListOfFiles()->FindObject(name);
  if (!fInputFile) {
    printf("AliTRDdigitizer::Open -- ");
    printf("Open the ALIROOT-file %s.\n",name);
    fInputFile = new TFile(name,"UPDATE");
  }
  else {
    printf("AliTRDdigitizer::Open -- ");
    printf("%s is already open.\n",name);
  }

  gAlice = (AliRun*) fInputFile->Get("gAlice");
  if (gAlice) {
    printf("AliTRDdigitizer::Open -- ");
    printf("AliRun object found on file.\n");
  }
  else {
    printf("AliTRDdigitizer::Open -- ");
    printf("Could not find AliRun object.\n");
    return kFALSE;
  }

  fEvent = nEvent;

  // Import the Trees for the event nEvent in the file
  Int_t nparticles = gAlice->GetEvent(fEvent);
  if (nparticles <= 0) {
    printf("AliTRDdigitizer::Open -- ");
    printf("No entries in the trees for event %d.\n",fEvent);
    return kFALSE;
  }

  // Get the pointer to the detector class and check for version 1
  fTRD = (AliTRD*) gAlice->GetDetector("TRD");
  if (fTRD->IsVersion() != 1) {
    printf("AliTRDdigitizer::Open -- ");
    printf("TRD must be version 1 (slow simulator).\n");
    exit(1);
  }

  // Get the geometry
  fGeo = fTRD->GetGeometry();
  printf("AliTRDdigitizer::Open -- ");
  printf("Geometry version %d\n",fGeo->IsVersion());

  return kTRUE;

}

//_____________________________________________________________________________
Bool_t AliTRDdigitizer::MakeDigits()
{
  //
  // Loops through the TRD-hits and creates the digits.
  //

  ///////////////////////////////////////////////////////////////
  // Parameter 
  ///////////////////////////////////////////////////////////////

  // Converts number of electrons to fC
  const Float_t kEl2fC  = 1.602E-19 * 1.0E15; 

  ///////////////////////////////////////////////////////////////

  Int_t   iRow, iCol, iTime;
  Int_t   nBytes = 0;
  Int_t   iDict;

  Int_t   totalSizeDigits = 0;
  Int_t   totalSizeDict0  = 0;
  Int_t   totalSizeDict1  = 0;
  Int_t   totalSizeDict2  = 0;

  AliTRDdataArrayI *digits;
  AliTRDdataArrayI *dictionary[kNDict];

  if (!fGeo) {
    printf("AliTRDdigitizer::MakeDigits -- ");
    printf("No geometry defined\n");
    return kFALSE;
  }

  // Create a digits manager
  fDigits = new AliTRDdigitsManager();

  // Create detector arrays to keep the signal and track numbers
  AliTRDdataArrayF *signal = new AliTRDdataArrayF();
  AliTRDdataArrayI *tracks[kNDict];
  for (iDict = 0; iDict < kNDict; iDict++) {
    tracks[iDict] = new AliTRDdataArrayI();
  }

  // Get the pointer to the hit tree
  TTree *hitTree = gAlice->TreeH();

  // Get the number of entries in the hit tree
  // (Number of primary particles creating a hit somewhere)
  Int_t nTrack = (Int_t) hitTree->GetEntries();

  printf("AliTRDdigitizer::MakeDigits -- ");
  printf("Start creating digits.\n");

  // The Lorentz factor
  if (fExBOn) {
    fLorentzFactor = 1.0 / (1.0 + fLorentzAngle*fLorentzAngle);
  }
  else {
    fLorentzFactor = 1.0;
  }

  Int_t chamBeg = 0;
  Int_t chamEnd = kNcham;
  if (fTRD->GetSensChamber()  >= 0) {
    chamBeg = fTRD->GetSensChamber();
    chamEnd = chamBeg + 1;
  }
  Int_t planBeg = 0;
  Int_t planEnd = kNplan;
  if (fTRD->GetSensPlane()    >= 0) {
    planBeg = fTRD->GetSensPlane();
    planEnd = planBeg + 1;
  }
  Int_t sectBeg = 0;
  Int_t sectEnd = kNsect;

  Int_t countHits = 0;

  // Loop through all the chambers
  for (Int_t iCham = chamBeg; iCham < chamEnd; iCham++) {
    for (Int_t iPlan = planBeg; iPlan < planEnd; iPlan++) {
      for (Int_t iSect = sectBeg; iSect < sectEnd; iSect++) {

        if (fTRD->GetSensSector() >= 0) {
          Int_t sens1 = fTRD->GetSensSector();
          Int_t sens2 = sens1 + fTRD->GetSensSectorRange();
          sens2 -= ((Int_t) (sens2 / kNsect)) * kNsect;
          if (sens1 < sens2) {
            if ((iSect < sens1) || (iSect >= sens2)) continue;
	  }
          else {
            if ((iSect < sens1) && (iSect >= sens2)) continue;
	  }
	}

        Int_t nDigits = 0;

        printf("AliTRDdigitizer::MakeDigits -- ");
        printf("Digitizing chamber %d, plane %d, sector %d.\n"
              ,iCham,iPlan,iSect);

        Int_t   iDet        = fGeo->GetDetector(iPlan,iCham,iSect);
        Int_t   nRowMax     = fGeo->GetRowMax(iPlan,iCham,iSect);
        Int_t   nColMax     = fGeo->GetColMax(iPlan);
        Int_t   nTimeMax    = fGeo->GetTimeMax();
        Float_t row0        = fGeo->GetRow0(iPlan,iCham,iSect);
        Float_t col0        = fGeo->GetCol0(iPlan);
        Float_t time0       = fGeo->GetTime0(iPlan);
        Float_t rowPadSize  = fGeo->GetRowPadSize();
        Float_t colPadSize  = fGeo->GetColPadSize();
        Float_t timeBinSize = fGeo->GetTimeBinSize();

        // Adjust the size of the detector arrays
        signal->Allocate(nRowMax,nColMax,nTimeMax);
        for (iDict = 0; iDict < kNDict; iDict++) {
          tracks[iDict]->Allocate(nRowMax,nColMax,nTimeMax);
	}

        // Loop through all entries in the tree
        for (Int_t iTrack = 0; iTrack < nTrack; iTrack++) {

          gAlice->ResetHits();
          nBytes += hitTree->GetEvent(iTrack);

          // Get the number of hits in the TRD created by this particle
          Int_t nHit = fTRD->Hits()->GetEntriesFast();

          // Loop through the TRD hits  
          for (Int_t iHit = 0; iHit < nHit; iHit++) {

            countHits++;

            AliTRDhit *hit = (AliTRDhit *) fTRD->Hits()->UncheckedAt(iHit);
            Float_t pos[3];
                    pos[0]   = hit->X();
                    pos[1]   = hit->Y();
                    pos[2]   = hit->Z();
            Float_t q        = hit->GetCharge();
            Int_t   track    = hit->Track();
            Int_t   detector = hit->GetDetector();
            Int_t   plane    = fGeo->GetPlane(detector);
            Int_t   sector   = fGeo->GetSector(detector);
            Int_t   chamber  = fGeo->GetChamber(detector);

            if ((sector  != iSect) ||
                (plane   != iPlan) ||
                (chamber != iCham)) 
              continue;

            // Rotate the sectors on top of each other
            Float_t rot[3];
            fGeo->Rotate(detector,pos,rot);

            // The hit position in pad coordinates (center pad)
            // The pad row (z-direction)
            Int_t  rowH = (Int_t) ((rot[2] -  row0) /  rowPadSize);
            // The pad column (rphi-direction)  
            Int_t  colH = (Int_t) ((rot[1] -  col0) /  colPadSize);
            // The time bucket
            Int_t timeH = (Int_t) ((rot[0] - time0) / timeBinSize);

            // Array to sum up the signal in a box surrounding the
            // hit postition
            const Int_t kTimeBox = 7;
            const Int_t  kColBox = 9;
            const Int_t  kRowBox = 7;
            Float_t signalSum[kRowBox][kColBox][kTimeBox];
            for (iRow  = 0;  iRow <  kRowBox; iRow++ ) {
              for (iCol  = 0;  iCol <  kColBox; iCol++ ) {
                for (iTime = 0; iTime < kTimeBox; iTime++) {
                  signalSum[iRow][iCol][iTime] = 0;
		}
	      }
	    }

            // Loop over all electrons of this hit
            Int_t nEl = (Int_t) q;
            for (Int_t iEl = 0; iEl < nEl; iEl++) {

	      // The driftlength
              Float_t driftlength = rot[0] - time0;
              if ((driftlength <        0) || 
                  (driftlength > kDrThick)) break;
              Float_t driftlengthL = driftlength;
              if (fExBOn) driftlengthL /= TMath::Sqrt(fLorentzFactor);
              Float_t xyz[3];
              xyz[0] = rot[0];
              xyz[1] = rot[1];
              xyz[2] = rot[2];

              // Electron attachment
              if (fElAttachOn) {
                if (gRandom->Rndm() < (driftlengthL * fElAttachProp / 100.)) continue;
	      }

              // Apply the diffusion smearing
              if (fDiffusionOn) {
                if (!(Diffusion(driftlengthL,xyz))) continue;
	      }

              // Apply E x B effects
              if (fExBOn) { 
                if (!(ExB(driftlength,xyz))) continue;   
	      }

              // The electron position and the distance to the hit position
	      // in pad units
              // The pad row (z-direction)
              Int_t  rowE = (Int_t) ((xyz[2] -  row0) /  rowPadSize);
              Int_t  rowD =  rowH -  rowE;
              // The pad column (rphi-direction)
              Int_t  colE = (Int_t) ((xyz[1] -  col0) /  colPadSize);
              Int_t  colD =  colH -  colE;
              // The time bucket
              Int_t timeE = (Int_t) ((xyz[0] - time0) / timeBinSize);
              Int_t timeD = timeH - timeE;

              // Apply the gas gain including fluctuations
              Int_t signal = (Int_t) (-fGasGain * TMath::Log(gRandom->Rndm()));

	      // The distance of the electron to the center of the pad 
	      // in units of pad width
              Float_t dist = (xyz[1] - col0 - (colE + 0.5) * colPadSize) 
                           / colPadSize;

              // Sum up the signal in the different pixels
              // and apply the pad response
              Int_t  rowIdx =  rowD + (Int_t) ( kRowBox / 2);
              Int_t  colIdx =  colD + (Int_t) ( kColBox / 2);
              Int_t timeIdx = timeD + (Int_t) (kTimeBox / 2);

              if (( rowIdx < 0) || ( rowIdx >  kRowBox)) {
                printf("AliTRDdigitizer::MakeDigits -- ");
                printf("Boundary error. rowIdx = %d (%d)\n", rowIdx, kRowBox);
                continue;
	      }
              if (( colIdx < 0) || ( colIdx >  kColBox)) {
                printf("AliTRDdigitizer::MakeDigits -- ");
                printf("Boundary error. colIdx = %d (%d)\n", colIdx, kColBox);
                continue;
	      }
              if ((timeIdx < 0) || (timeIdx > kTimeBox)) {
                printf("AliTRDdigitizer::MakeDigits -- ");
                printf("Boundary error. timeIdx = %d (%d)\n",timeIdx,kTimeBox);
                continue;
	      }
              signalSum[rowIdx][colIdx-1][timeIdx] += fPRF->Eval(dist-1.0,0,0) * signal;
              signalSum[rowIdx][colIdx  ][timeIdx] += fPRF->Eval(dist    ,0,0) * signal;
              signalSum[rowIdx][colIdx+1][timeIdx] += fPRF->Eval(dist+1.0,0,0) * signal;

            }

            // Add the padcluster to the detector matrix
            for (iRow  = 0;  iRow <  kRowBox; iRow++ ) {
              for (iCol  = 0;  iCol <  kColBox; iCol++ ) {
                for (iTime = 0; iTime < kTimeBox; iTime++) {

                  Int_t  rowB =  rowH + iRow  - (Int_t) ( kRowBox / 2); 
                  Int_t  colB =  colH + iCol  - (Int_t) ( kColBox / 2);
                  Int_t timeB = timeH + iTime - (Int_t) (kTimeBox / 2);
                  Float_t signalB = signalSum[iRow][iCol][iTime];
                  if (( rowB < 0) || ( rowB >=  nRowMax)) continue;
                  if (( colB < 0) || ( colB >=  nColMax)) continue;
                  if ((timeB < 0) || (timeB >= nTimeMax)) continue;
                  if (signalB > 0.0) {

                    // Add the signal sum  
                    signalB += signal->GetData(rowB,colB,timeB);
                    signal->SetData(rowB,colB,timeB,signalB);  
                    // Store the track index in the dictionary
                    // Note: We store index+1 in order to allow the array to be compressed
                    for (iDict = 0; iDict < kNDict; iDict++) {
                      Int_t oldTrack = tracks[iDict]->GetData(rowB,colB,timeB);
                      if (oldTrack == track+1) break;
                      if (oldTrack ==      -1) break;
                      if (oldTrack ==       0) {
                        tracks[iDict]->SetData(rowB,colB,timeB,track+1);
                        break;
                      }
                    }
                    if (iDict == kNDict) {
                      printf("AliTRDdigitizer::MakeDigits -- ");
                      printf("More than three tracks for one digit!\n");
                    }
 		  }

		}
	      }
	    }

          }

 	}

        // Add a container for the digits of this detector
        digits = fDigits->GetDigits(iDet);        
        // Allocate memory space for the digits buffer
        digits->Allocate(nRowMax,nColMax,nTimeMax);

	// Do the same for the dictionary arrays
        for (iDict = 0; iDict < kNDict; iDict++) {
          dictionary[iDict] = fDigits->GetDictionary(iDet,iDict);
          dictionary[iDict]->Allocate(nRowMax,nColMax,nTimeMax);
	}

        // Create the digits for this chamber
        for (iRow  = 0; iRow  <  nRowMax; iRow++ ) {
          for (iCol  = 0; iCol  <  nColMax; iCol++ ) {
            for (iTime = 0; iTime < nTimeMax; iTime++) {         

              Float_t signalAmp = signal->GetData(iRow,iCol,iTime);

              // Add the noise
              signalAmp  = TMath::Max((Float_t) gRandom->Gaus(signalAmp,fNoise)
                                     ,(Float_t) 0.0);
	      // Convert to fC
              signalAmp *= kEl2fC;
              // Convert to mV
              signalAmp *= fChipGain;
	      // Convert to ADC counts
              Int_t adc  = (Int_t) (signalAmp * (fADCoutRange / fADCinRange));

              if (adc > fADCthreshold) {

                nDigits++;

                // Store the amplitude of the digit
                digits->SetData(iRow,iCol,iTime,adc);

                // Store the track index in the dictionary
                // Note: We store index+1 in order to allow the array to be compressed
                for (iDict = 0; iDict < kNDict; iDict++) {
                  dictionary[iDict]->SetData(iRow,iCol,iTime
                                            ,tracks[iDict]->GetData(iRow,iCol,iTime));
	        }

	      }

	    }
	  }
	}

        // Compress the arrays
        digits->Compress(1,0);
        for (iDict = 0; iDict < kNDict; iDict++) {
          dictionary[iDict]->Compress(1,0);
	}

        totalSizeDigits += digits->GetSize();
        totalSizeDict0  += dictionary[0]->GetSize();
        totalSizeDict1  += dictionary[1]->GetSize();
        totalSizeDict2  += dictionary[2]->GetSize();

        printf("AliTRDdigitizer::MakeDigits -- ");
        printf("Number of digits found: %d.\n",nDigits);
 
	// Reset the arrays
        signal->Reset();
        for (iDict = 0; iDict < kNDict; iDict++) {
          tracks[iDict]->Reset();
	}

      }
    }
  }

  printf("AliTRDdigitizer::MakeDigits -- ");
  printf("Total number of analyzed hits = %d\n",countHits);

  printf("AliTRDdigitizer::MakeDigits -- ");
  printf("Total digits data size = %d, %d, %d, %d\n",totalSizeDigits
                                                    ,totalSizeDict0
                                                    ,totalSizeDict1
                                                    ,totalSizeDict2);        

  return kTRUE;

}

//_____________________________________________________________________________
Bool_t AliTRDdigitizer::WriteDigits()
{
  //
  // Writes out the TRD-digits and the dictionaries
  //

  // Create the branches
  if (!(gAlice->TreeD()->GetBranch("TRDdigits"))) { 
    if (!fDigits->MakeBranch()) return kFALSE;
  }

  // Store the digits and the dictionary in the tree
  fDigits->WriteDigits();

  // Write the new tree into the input file (use overwrite option)
  Char_t treeName[7];
  sprintf(treeName,"TreeD%d",fEvent);
  printf("AliTRDdigitizer::WriteDigits -- ");
  printf("Write the digits tree %s for event %d.\n"
        ,treeName,fEvent);
  gAlice->TreeD()->Write(treeName,2);
 
  return kTRUE;

}
