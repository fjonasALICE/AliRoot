// @(#) $Id: AliHLTTPCCATrackletSelector.cxx 27042 2008-07-02 12:06:02Z richterm $
// **************************************************************************
// This file is property of and copyright by the ALICE HLT Project          *
// ALICE Experiment at CERN, All rights reserved.                           *
//                                                                          *
// Primary Authors: Sergey Gorbunov <sergey.gorbunov@kip.uni-heidelberg.de> *
//                  Ivan Kisel <kisel@kip.uni-heidelberg.de>                *
//                  for The ALICE HLT Project.                              *
//                                                                          *
// Permission to use, copy, modify and distribute this software and its     *
// documentation strictly for non-commercial purposes is hereby granted     *
// without fee, provided that the above copyright notice appears in all     *
// copies and that both the copyright notice and this permission notice     *
// appear in the supporting documentation. The authors make no claims       *
// about the suitability of this software for any purpose. It is            *
// provided "as is" without express or implied warranty.                    *
//                                                                          *
//***************************************************************************


#include "AliHLTTPCCATrackletSelector.h"
#include "AliHLTTPCCATrack.h"
#include "AliHLTTPCCATracker.h"
#include "AliHLTTPCCATrackParam.h"
#include "AliHLTTPCCATracklet.h"
#include "AliHLTTPCCAMath.h"

GPUd() void AliHLTTPCCATrackletSelector::Thread
( int nBlocks, int nThreads, int iBlock, int iThread, int iSync,
  AliHLTTPCCASharedMemory &s, AliHLTTPCCATracker &tracker )
{
  // select best tracklets and kill clones

  if ( iSync == 0 ) {
    if ( iThread == 0 ) {
      s.fNTracklets = *tracker.NTracklets();
      s.fNThreadsTotal = nThreads * nBlocks;
      s.fItr0 = nThreads * iBlock;
    }
  } else if ( iSync == 1 ) {
	int nHits, nFirstTrackHit;
	AliHLTTPCCAHitId trackHits[160 - HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE];

    for ( int itr = s.fItr0 + iThread; itr < s.fNTracklets; itr += s.fNThreadsTotal ) {

#ifdef HLTCA_GPU_EMULATION_DEBUG_TRACKLET
	if (itr == HLTCA_GPU_EMULATION_DEBUG_TRACKLET)
	{
		tracker.GPUParameters()->fGPUSchedCollisions += 1;
	}
#endif

	  while (tracker.Tracklets()[itr].NHits() == 0)
	  {
		  itr += s.fNThreadsTotal;
		  if (itr >= s.fNTracklets) return;
	  }

	  AliHLTTPCCATracklet &tracklet = tracker.Tracklets()[itr];
      const int tNHits = tracklet.NHits();

      const int kMaxRowGap = 4;
      const float kMaxShared = .1;

      int firstRow = tracklet.FirstRow();
      int lastRow = tracklet.LastRow();

      int kind = 0;
      if ( 0 ) {
        if ( tNHits >= 10 && 1. / .5 >= CAMath::Abs( tracklet.Param().QPt() ) ) { //SG!!!
          kind = 1;
        }
      }

      int w = ( kind << 29 ) + ( tNHits << 16 ) + itr;

      //int w = (tNHits<<16)+itr;
      //int nRows = tracker.Param().NRows();
      //std::cout<<" store tracklet: "<<firstRow<<" "<<lastRow<<std::endl;

	  int irow = firstRow;

      int gap = 0;
      int nShared = 0;
	  nHits = 0;

	  for (irow = firstRow; irow <= lastRow && lastRow - irow + nHits >= TRACKLET_SELECTOR_MIN_HITS; irow++ ) {
        gap++;
        int ih = tracklet.RowHit( irow );
        if ( ih >= 0 ) {
          const AliHLTTPCCARow &row = tracker.Row( irow );
          bool own = ( tracker.HitWeight( row, ih ) <= w );
          bool sharedOK = ( ( nShared < nHits * kMaxShared ) );
          if ( own || sharedOK ) {//SG!!!
            gap = 0;
#if HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE != 0
			if (nHits < HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE)
				s.fHits[iThread][nHits].Set( irow, ih );
			else
#endif
				trackHits[nHits - HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE].Set( irow, ih );
            nHits++;
            if ( !own ) nShared++;
          }
        }

        if ( gap > kMaxRowGap || irow == lastRow ) { // store
          if ( nHits >= TRACKLET_SELECTOR_MIN_HITS ) { //SG!!!
            int itrout = CAMath::AtomicAdd( tracker.NTracks(), 1 );
            nFirstTrackHit = CAMath::AtomicAdd( tracker.NTrackHits(), nHits );
            /*tout.SetParam( tracklet.Param() );
            tout.SetAlive( 1 );
            tracker.Tracks()[itrout] = tout;*/
			tracker.Tracks()[itrout].SetAlive(1);
			tracker.Tracks()[itrout].SetParam(tracklet.Param());
			tracker.Tracks()[itrout].SetFirstHitID(nFirstTrackHit);
			tracker.Tracks()[itrout].SetNHits(nHits);
            for ( int jh = 0; jh < nHits; jh++ ) {
#if HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE != 0
				if (jh < HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE)
					tracker.TrackHits()[nFirstTrackHit + jh] = s.fHits[iThread][jh];
				else
#endif
					tracker.TrackHits()[nFirstTrackHit + jh] = trackHits[jh - HLTCA_GPU_TRACKLET_SELECTOR_HITS_REG_SIZE];
            }
          }
          nHits = 0;
          gap = 0;
          nShared = 0;
        }
      }


    }
  }
}