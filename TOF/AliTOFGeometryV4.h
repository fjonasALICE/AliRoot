#ifndef ALITOFGEOMETRYV4_H
#define ALITOFGEOMETRYV4_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  TOF geometry class (PPR version)                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "AliTOFGeometry.h"

class AliTOFGeometryV4: public AliTOFGeometry {

 public:
  AliTOFGeometryV4();
  virtual ~AliTOFGeometryV4();
  
  void    Init();
  Bool_t  IsInsideThePad(Int_t *det, Float_t *pos);
  Float_t DistanceToPad(Int_t *det, Float_t *pos, Float_t *dist3d=0);
  Int_t   GetPlate(Float_t *pos);
  Int_t   GetStrip(Float_t *pos);
  Int_t   GetSector(Float_t *pos);
  Int_t   GetPadX(Float_t *pos);
  Int_t   GetPadZ(Float_t *pos);
  Float_t GetX(Int_t *det);
  Float_t GetY(Int_t *det);
  Float_t GetZ(Int_t *det);
  Float_t GetMinPlateTheta(Int_t iPlate);
  Float_t GetMaxPlateTheta(Int_t iPlate);
  Float_t GetMinStripTheta(Int_t iPlate, Int_t iStrip);
  Float_t GetMaxStripTheta(Int_t iPlate, Int_t iStrip);
  Float_t GetStripTheta(Int_t iPlate, Int_t iStrip);

  Float_t NStirpC()     { return kNStripC;};
  Int_t   NPadXSector() { return (AliTOFGeometry::kNStripA + 2*AliTOFGeometry::kNStripB +
				  2*kNStripC)*AliTOFGeometry::kNpadX*AliTOFGeometry::kNpadZ;};

  Float_t RinTOF()      { return fgkxTOF;};
  Float_t Rmin()        { return fgkRmin;};
  Float_t Rmax()        { return fgkRmax;};

  Float_t ZlenA()       { return fgkZlenA;};
  Float_t ZlenB()       { return fgkZlenB;};
  Float_t ZlenC()       { return fgkZlenC;};
  Float_t MaxhZtof()    { return fgkMaxhZtof;};

  static  Float_t DeadBndX()    { return fgkDeadBndX;};
  static  Float_t DeadBndZ()    { return fgkDeadBndZ;};
  static  Float_t OverSpc()     { return fgkOverSpc;};

  protected:

  //private:

  static const Int_t kNStripC;         // number of strips in C type module 

  static const Float_t fgkZlenA;       // length (cm) of the A module
  static const Float_t fgkZlenB;       // length (cm) of the B module
  static const Float_t fgkZlenC;       // length (cm) of the C module
  static const Float_t fgkMaxhZtof;    // Max half z-size of TOF (cm)

  static const Float_t fgkRmin;        // Inner radius of the TOF (cm)
  static const Float_t fgkRmax;        // Outer radius of the TOF (cm)
  static const Float_t fgkxTOF;        // Inner TOF Radius used in Reconstruction (cm)

  static const Float_t fgkDeadBndX; // Dead Boundaries of a Strip along Z direction (width)
  static const Float_t fgkDeadBndZ; // Dead Boundaries of a Strip along X direction (length)
  static const Float_t fgkOverSpc;  // Space available for sensitive layers in radial direction (cm)

  static const Float_t fgkDprecMin;//num.prec.tolerance on Thmin 
  static const Float_t fgkDprecMax;//num.prec.tolerance on Thma 
  static const Float_t fgkDprecCen;//num.prec.tolerance on <Theta> 

  ClassDef(AliTOFGeometryV4,1) // TOF Geometry class
};

#endif
