/*************************************************************************
 *
 *  I have modified the CAEN routines to decode DT5780 events in order 
 *  to be able to work without opening connection to Digitizer 
 *
 ************************************************************************/

uint32_t extendedTimestamp[2]={0,0};

static CAEN_DGTZ_ErrorCode CAENDGTZ_API CAEN_DGTZ_DecodeDPPAggregate(int ch, uint32_t *data, CAEN_DGTZ_DPP_PHA_Event_t *events, uint32_t *numEventsCh) {
  uint32_t i,size, evsize, nev, et, ee, ew, pnt,dec;
  uint64_t TMPtt = 0;

  if (!(data[0] & 0x80000000))
    return CAEN_DGTZ_InvalidEvent;

  size = data[0] & 0x7FFFFFFF;
  et = (data[1]>>28) & 1;
  ee = (data[1]>>29) & 1;
  ew = (data[1]>>30) & 1;
  dec = (data[1]>>24) & 0x3;
  evsize = (data[1] & 0xFFFF) + et + ee;

  if(evsize==0)
    return CAEN_DGTZ_InvalidEvent;
  if ((size - 2) % evsize)
    return CAEN_DGTZ_InvalidEvent;
  nev = (size - 2) / evsize;
  pnt = 2;
  for (i=0; i<nev; i++) {
    events[i].Format = data[1];
    events[i].Extras2 = 0; // NOTE: Extras2 word exists only for x730 boards.
    if (et) {
      TMPtt = data[pnt++] & 0x3FFFFFFF;
    }
    if (ew) {
      events[i].Waveforms = data + pnt;
      pnt += data[1] & 0xFFFF ;
    }
    else
      events[i].Waveforms = NULL;
    if (ee) {
      events[i].Energy = data[pnt] & 0xFFFF; // NOTE: keep pileup bit
      events[i].Extras = (data[pnt]>>16) & 0xFFFF;

      // TTRESET MANAGEMENT
      if (events[i].Extras & 0x00000004) // TTRESET
	extendedTimestamp[ch] = 0;
	
      if (events[i].Extras & 0x00000002) // TTROLLOVER
	extendedTimestamp[ch]++;
	
      events[i].TimeTag = extendedTimestamp[ch];
      events[i].TimeTag = events[i].TimeTag << 30;  //if TTROLLOVER TimeTag=2^30
      events[i].TimeTag += TMPtt; // time in samples
      events[i].TimeTag = events[i].TimeTag * (uint64_t)(1 << dec); // take into account decimation
      pnt++;       // Increment pointer.
    }
  }
  *numEventsCh = nev;
  
  return CAEN_DGTZ_Success;
}


CAEN_DGTZ_ErrorCode DT5780_DPP_PHA_GetDPPEvents(char *buffer, uint32_t buffsize, CAEN_DGTZ_DPP_PHA_Event_t **events, uint32_t* numEventsArray) {
    uint32_t *buffer32, endaggr;
    int ch; 
    uint32_t pnt=0, nevch;
    char chmask;
    int maxch=2; //MaxChannels;
    int* index = (int *)calloc(maxch, sizeof(int));
    
    buffer32 = (uint32_t *)buffer; 
    while (pnt < (buffsize / 4) - 4) { // the difference must be at least the header size (4).
    	if ((buffer32[pnt] >> 28) != 0xA) { // invalid header signature
    		free(index);
    		return CAEN_DGTZ_InvalidEvent;
    	}
        endaggr = pnt + (buffer32[pnt] & 0x0FFFFFFF);
        chmask = (char)(buffer32[pnt+1] & 0xFF);
        pnt += 4;
        if (chmask == 0)
            continue;
        for(ch=0; ch<maxch; ch++) {
            if (!(chmask & (1<<ch)))
                continue;

            if (CAEN_DGTZ_DecodeDPPAggregate(ch, buffer32+pnt, events[ch]+index[ch], &nevch)) {
                free(index);
                return CAEN_DGTZ_InvalidEvent;
            }
            index[ch] += nevch;
            pnt += buffer32[pnt] & 0x7FFFFFFF;
        }    
        if (pnt != endaggr) {
            free(index);
            return CAEN_DGTZ_InvalidEvent;
        }
    }
    for(ch=0; ch<maxch; ch++)
        numEventsArray[ch] = index[ch];

    free(index);
    return CAEN_DGTZ_Success;
}

CAEN_DGTZ_ErrorCode DT5780_DPP_PHA_DecodeDPPWaveforms(CAEN_DGTZ_DPP_PHA_Event_t *event, CAEN_DGTZ_DPP_PHA_Waveforms_t *waveforms) {


    int i;
    waveforms->Ns = (event->Format & 0xFFFF) * 2;
    waveforms->VProbe1 = (uint8_t)((event->Format>>22) & 0x3);
    waveforms->VProbe2 = (uint8_t)((event->Format>>20) & 0x3);
    waveforms->VDProbe = (uint8_t)((event->Format>>16) & 0xF);
    waveforms->DualTrace = (uint8_t)((event->Format>>31) & 0x1);
    for(i=0; i<(int)(waveforms->Ns/2); i++) {
        int16_t T1S1, T1S2, T2S1, T2S2;

        if (!waveforms->DualTrace) {
            T1S1 = (int16_t)(event->Waveforms[i] & 0x3FFF);
            T1S2 = (int16_t)((event->Waveforms[i]>>16) & 0x3FFF);
            T2S1 = 0;
            T2S2 = 0;
        } else {
            T1S1 = (int16_t)((event->Waveforms[i]>>16) & 0x3FFF);
            T1S2 = T1S1;
            T2S1 = (int16_t)((event->Waveforms[i]) & 0x3FFF);
            T2S2 = T2S1;
        }

        // Extend the sign bit (bit 13)
        if (waveforms->VProbe1 != CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Input) {
            if ((T1S1 >> 13) & 0x1)
                T1S1 |= 0xC000; // Set sign bits
            if ((T1S2 >> 13) & 0x1)
                T1S2 |= 0xC000; // Set sign bits
        }
        if ((waveforms->VProbe1 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Delta) ||
            (waveforms->VProbe1 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Delta2))
        { // Delta and Delta2 probes are divided by 2 to let the sign bit to take place
            T1S1 <<= 1;
            T1S2 <<= 1;
        } else if (waveforms->VProbe1 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_trapezoid) {
            // Trapezoid-related quantyties are divided by 4 (it is calculated on
            // 15 bit + sign, but the word is 14 bits. So the sample is shifted by 2)
            T1S1 <<= 2;
            T1S2 <<= 2;
        }
        // Extend the sign bit (bit 13)
        if (waveforms->DualTrace &&
            (waveforms->VProbe2 != CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_Input)) {
            if ((T2S1 >> 13) & 0x1)
                T2S1 |= 0xC000; // Set sign bits
            if ((T2S2 >> 13) & 0x1)
                T2S2 |= 0xC000; // Set sign bits

            if (waveforms->VProbe2 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_S3) {
                // S3 probe is divided by 2 to let the sign bit to take place
                T2S1 <<= 1;
                T2S2 <<= 1;
            } else if (
                (waveforms->VProbe2 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_DigitalCombo) ||
                (waveforms->VProbe2 == CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_trapBaseline))
            {
                // Trapezoid-related quantyties are divided by 4 (it is calculated on
                // 15 bit + sign, but the word is 14 bits. So the sample is shifted by 2)
                T2S1 <<= 2;
                T2S2 <<= 2;
            }
        }
        // Assign Analog Samples
	
        waveforms->Trace1[i*2] = T1S1;
        waveforms->Trace1[i*2+1] = T1S2;
        waveforms->Trace2[i*2] = T2S1;
        waveforms->Trace2[i*2+1] = T2S2;
        // Assign Digital Samples
        waveforms->DTrace1[i*2] = (uint8_t)((event->Waveforms[i]>>14) & 1);
        waveforms->DTrace1[i*2+1] = (uint8_t)((event->Waveforms[i]>>30) & 1);
        waveforms->DTrace2[i*2] = (uint8_t)((event->Waveforms[i]>>15) & 1);
        waveforms->DTrace2[i*2+1] = (uint8_t)((event->Waveforms[i]>>31) & 1);
    }    
    return CAEN_DGTZ_Success;
}
