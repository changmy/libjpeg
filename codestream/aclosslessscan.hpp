/*
**
** Represents the lossless scan - lines are coded directly with predictive
** coding, though here residuals are encoded with the arithmetic encoder.
**
** $Id: aclosslessscan.hpp,v 1.28 2014/11/16 15:49:58 thor Exp $
**
*/

#ifndef CODESTREAM_ACLOSSLESSSCAN_HPP
#define CODESTREAM_ACLOSSLESSSCAN_HPP

/// Includes
#include "tools/environment.hpp"
#include "coding/qmcoder.hpp"
#include "codestream/entropyparser.hpp"
#include "codestream/predictivescan.hpp"
///

/// Forwards
class Frame;
class LineCtrl;
class ByteStream;
class LineBitmapRequester;
class LineBuffer;
class BitmaCtrl;
class Scan;
///

/// class LosslessScan
// Represents the lossless scan - lines are coded directly with predictive
// coding, though here residuals are encoded with the arithmetic encoder.
class ACLosslessScan : public PredictiveScan {
  //
#if ACCUSOFT_CODE
  // The class used for pulling and pushing data.
  class LineBuffer          *m_pLineCtrl;
  //
  // Small DC threshold value ('L' in the standard)
  UBYTE                      m_ucSmall[4];
  //
  // Large DC threshold value ('U' in the specs)
  UBYTE                      m_ucLarge[4];
  //
  // The context index to use.
  UBYTE                      m_ucContext[4];
  //
  // Differentials from the above and left, used
  // for prediction.
  LONG                      *m_plDa[4];
  LONG                      *m_plDb[4];
  //
  // The real worker class.
  class QMCoder              m_Coder;
  //
  // Context information.
  struct QMContextSet {
    //
    // The Zero-Sign coding contexts - this is a 5x5 set.
    struct ContextZeroSet {
      QMContext S0,SS,SP,SN;
      //
      void Init(void)
      {
	S0.Init();
	SS.Init();
	SP.Init();
	SN.Init();
      }
    } SignZeroCoding[5][5];
    //
    // The Magnitude/refinement coding contexts.
    struct MagnitudeSet {
      QMContext X[15];
      QMContext M[15];
      //
      void Init(void)
      {
	for(int i = 0;i < 15;i++) {
	  X[i].Init();
	  M[i].Init();
	}
      }
    } MagnitudeLow,MagnitudeHigh;
    //
    void Init(void)
    {
      for(int i = 0;i <5;i++) {
	for(int j = 0;j < 5;j++) {
	  SignZeroCoding[i][j].Init();
	}
      }
      MagnitudeLow.Init();
      MagnitudeHigh.Init();
    }
    //
    // Classify and return the sign/zero coding context to encode the difference in.
    // Requires the differences in both directions.
    struct ContextZeroSet &ClassifySignZero(LONG Da,LONG Db,UBYTE l,UBYTE u)
    {
      return SignZeroCoding[Classify(Da,l,u) + 2][Classify(Db,l,u) + 2];
    }
    //
    // Classify the Magnitude context 
    struct MagnitudeSet &ClassifyMagnitude(LONG Db,UBYTE u)
    {
      if (Db > (1 << u) || -Db > (1 << u)) {
	return MagnitudeHigh;
      } else {
	return MagnitudeLow;
      }
    }
    //
    // Classifier in one direction.
    static int Classify(LONG diff,UBYTE l,UBYTE u)
    {
      LONG abs = (diff > 0)?(diff):(-diff);
  
      if (abs <= ((1 << l) >> 1)) {
	// the zero cathegory.
	return 0;
      }
      if (abs <= (1 << u)) {
	if (diff < 0) {
	  return -1;
	} else {
	  return 1;
	}
      }
      if (diff < 0) {
	return -2;
      } else {
	return 2;
      }
    }
    //
  } m_Context[4];
  //
  // Common setup for encoding and decoding.
  void FindComponentDimensions(void);
  //
#endif
  // This is actually the true MCU-parser, not the interface that reads
  // a full line.
  void ParseMCU(struct Line **prev,struct Line **top);
  //
  // The actual MCU-writer, write a single group of pixels to the stream,
  // or measure their statistics.
  void WriteMCU(struct Line **prev,struct Line **top);
  //
  // Flush the remaining bits out to the stream on writing.
  virtual void Flush(bool final); 
  // 
  // Restart the parser at the next restart interval
  virtual void Restart(void);
  //
  //
public:
  ACLosslessScan(class Frame *frame,class Scan *scan,UBYTE predictor,UBYTE lobit,
		 bool differential = false);
  //
  virtual ~ACLosslessScan(void);
  //
  // Write the marker that indicates the frame type fitting to this scan.
  virtual void WriteFrameType(class ByteStream *io);
  //
  // Fill in the tables for decoding and decoding parameters in general.
  virtual void StartParseScan(class ByteStream *io,class Checksum *chk,class BufferCtrl *ctrl);
  //
  // Write the default tables for encoding
  virtual void StartWriteScan(class ByteStream *io,class Checksum *chk,class BufferCtrl *ctrl);
  //
  // Start the measurement run - not required here.
  virtual void StartMeasureScan(class BufferCtrl *ctrl);
  //
  // Start a MCU scan. Returns true if there are more rows. False otherwise.
  // Note that we emulate here that MCUs are multiples of eight lines high
  // even though from a JPEG perspective a MCU is a single pixel in the
  // lossless coding case.
  virtual bool StartMCURow(void);
  //
  // Parse a single MCU in this scan. Return true if there are more
  // MCUs in this row.
  virtual bool ParseMCU(void);
  //
  // Write a single MCU in this scan.
  virtual bool WriteMCU(void);  
};
///


///
#endif
