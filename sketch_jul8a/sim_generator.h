#pragma once
#include <Arduino.h>

enum SimState { NORMAL, SPIKE, OUTAGE, WARMUP };

struct SimParams {
  float base, noise, spikeMin, spikeMax;
  uint32_t periodMs, spikeMs, intervalMs, warmupMs;
  float dropoutProb;
  uint8_t plateauN, startupSamples;
};

class SimGenerator {
 public:
  SimGenerator(uint32_t seed, const SimParams& p): params(p) { randomSeed(seed); reset(); }
  void reset(){ t0=millis(); last=0; lastChg=t0; state=(params.warmupMs?WARMUP:NORMAL); plate=0; seq=0; val=NAN; off=0; out=false; }
  bool tick(){
    uint32_t now=millis(); if (now-last<params.intervalMs) return false;
    if(state!=SPIKE && ((now-t0)%params.periodMs)<params.intervalMs){ state=SPIKE; lastChg=now; off=randf(params.spikeMin, params.spikeMax); }
    if(state==SPIKE && (now-lastChg)>=params.spikeMs){ state=NORMAL; lastChg=now; }
    if(state==WARMUP && (now-t0)>=params.warmupMs){ state=NORMAL; lastChg=now; }
    out = (randf(0,1) < params.dropoutProb);
    if(!out){
      if(plate==0){
        float lvl = params.base + ((state==SPIKE)? off : 0);
        val = lvl + randf(-params.noise, params.noise);
        plate = max<uint8_t>(1, params.plateauN) - 1;
      } else plate--;
    }
    last=now; seq++; return true;
  }
  float value() const { return out ? NAN : val; }
  bool outage() const { return out; }
  bool startup() const { return seq <= params.startupSamples; }
 private:
  SimParams params; uint32_t t0=0,last=0,lastChg=0; SimState state=NORMAL; uint8_t plate=0; float val=NAN, off=0; bool out=false; uint32_t seq=0;
  float randf(float a,float b) const { long r=random(0,10000); return a+(b-a)*(r/10000.0f); }
};