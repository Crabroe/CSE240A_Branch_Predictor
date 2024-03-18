//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Yuhuan Chen";
const char *studentID   = "A59023864";
const char *email       = "yuc178@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//Gshare
uint32_t ghistory;          // Global history register
uint8_t *gshareBHT;         // Branch History Table
//Tournament
uint32_t *localHistoryTable; // Local History Table
uint8_t *localBHT;           // Local Branch History Table
uint8_t *globalBHT;          // Global Branch History Table
uint8_t *choiceBHT;          // Choice Predictor Table
//Custom

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor() {
  switch (bpType) {
    case GSHARE:
      gshareBHT = (uint8_t *)malloc((1 << ghistoryBits) * sizeof(uint8_t));
      for (int i = 0; i < 1 << ghistoryBits; i++) {
        gshareBHT[i] = WN;
      }
      ghistory = 0;
      break;
    
    case TOURNAMENT:
      localHistoryTable = (uint32_t *)malloc((1 << pcIndexBits) * sizeof(uint32_t));
      localBHT = (uint8_t *)malloc((1 << lhistoryBits) * sizeof(uint8_t));
      globalBHT = (uint8_t *)malloc((1 << ghistoryBits) * sizeof(uint8_t));
      choiceBHT = (uint8_t *)malloc((1 << ghistoryBits) * sizeof(uint8_t));
      for (int i = 0; i < (1 << pcIndexBits); i++) {
        localHistoryTable[i] = 0;
      }
      for (int i = 0; i < (1 << lhistoryBits); i++) {
        localBHT[i] = WN;
      }
      for (int i = 0; i < (1 << ghistoryBits); i++) {
        globalBHT[i] = WN;
        choiceBHT[i] = WN;
      }
      ghistory = 0;
      break;
    
    case CUSTOM:
      localHistoryTable = (uint32_t *)malloc((1 << pcIndexBits) * sizeof(uint32_t));
      localBHT = (uint8_t *)malloc((1 << lhistoryBits) * sizeof(uint8_t));
      globalBHT = (uint8_t *)malloc((1 << ghistoryBits) * sizeof(uint8_t));
      choiceBHT = (uint8_t *)malloc((1 << ghistoryBits) * sizeof(uint8_t));
      for (int i = 0; i < (1 << pcIndexBits); i++) {
        localHistoryTable[i] = 0;
      }
      for (int i = 0; i < (1 << lhistoryBits); i++) {
        localBHT[i] = WN;
      }
      for (int i = 0; i < (1 << ghistoryBits); i++) {
        globalBHT[i] = WN;
        choiceBHT[i] = WN;
      }
      ghistory = 0;
      break;
    default:
      break;
  }
}


// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc) {
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE: {
      uint32_t xor=pc ^ ghistory;
      uint32_t ID = xor&((1 << ghistoryBits) - 1);
      if(gshareBHT[ID] >= WT){return TAKEN;}
      else{return NOTTAKEN;}
    }
    case TOURNAMENT: {
      uint32_t localID = pc & ((1 << pcIndexBits) - 1);
      uint32_t localHistory = localHistoryTable[localID];

      uint32_t localBHTID = localHistory & ((1 << lhistoryBits) - 1);
      uint8_t localPrediction;

      if(localBHT[localBHTID] >= WT){localPrediction=TAKEN;}
      else{localPrediction=NOTTAKEN;}

      uint32_t globalHistory = ghistory & ((1 << ghistoryBits) - 1);
      uint8_t globalPrediction; 

      if(globalBHT[globalHistory] >= WT){globalPrediction=TAKEN;}
      else{globalPrediction=NOTTAKEN;}

      uint32_t choiceID = globalHistory;
      uint8_t choice = choiceBHT[choiceID];

      if (choice >= WT) {
        return globalPrediction;
      } else {
        return localPrediction;
      }
    }
    case CUSTOM: {
      uint32_t localID = pc & ((1 << pcIndexBits) - 1);
      uint32_t localHistory = localHistoryTable[localID];

      uint32_t localBHTID = localHistory & ((1 << lhistoryBits) - 1);
      uint8_t localPrediction;

      if(localBHT[localBHTID] >= WT){localPrediction=TAKEN;}
      else{localPrediction=NOTTAKEN;}

      uint32_t globalHistory = (pc ^ ghistory) & ((1 << ghistoryBits) - 1);
      uint8_t globalPrediction; 

      if(globalBHT[globalHistory] >= WT){globalPrediction=TAKEN;}
      else{globalPrediction=NOTTAKEN;}

      uint32_t choiceID = globalHistory;
      uint8_t choice = choiceBHT[choiceID];

      if (choice >= WT) {
        return globalPrediction;
      } else {
        return localPrediction;
      }
    }
    default:
      return NOTTAKEN;
  }
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome) {
  switch (bpType) {
    case GSHARE: {
      uint32_t xor=pc ^ ghistory;
      uint32_t ID = xor&((1 << ghistoryBits) - 1);
      if (outcome == TAKEN) {
        if(gshareBHT[ID] == ST){gshareBHT[ID] =ST;}
        else{gshareBHT[ID] =gshareBHT[ID] + 1;}
      } 
      else {
        if(gshareBHT[ID] == SN){gshareBHT[ID] = SN;}
        else{gshareBHT[ID] = gshareBHT[ID] - 1;}
      }
      ghistory = ((ghistory << 1) | outcome) & ((1 << ghistoryBits) - 1);
      break;
    }
    case TOURNAMENT: {
      uint32_t localID = pc & ((1 << pcIndexBits) - 1);
      uint32_t localHistory = localHistoryTable[localID];
      uint32_t localBHTID = localHistory & ((1 << lhistoryBits) - 1);

      if (outcome == TAKEN) {
        if(localBHT[localBHTID] == ST){localBHT[localBHTID]=ST;}
        else{localBHT[localBHTID]=localBHT[localBHTID] + 1;}
      } 
      else {
        if(localBHT[localBHTID] == SN){localBHT[localBHTID] = SN;}
        else{localBHT[localBHTID] = localBHT[localBHTID] - 1;}
      }

      uint32_t globalHistory = ghistory & ((1 << ghistoryBits) - 1);

      if (outcome == TAKEN) {
        if(globalBHT[globalHistory] == ST){globalBHT[globalHistory] = ST;}
        else{globalBHT[globalHistory] = globalBHT[globalHistory] + 1;}
      } else {
        if(globalBHT[globalHistory] == SN){globalBHT[globalHistory] = SN;}
        else{globalBHT[globalHistory] = globalBHT[globalHistory] - 1;}
      }

      uint8_t localPrediction;
      uint8_t globalPrediction;

      if(localBHT[localBHTID] >= WT){localPrediction=TAKEN;}
      else{localPrediction=NOTTAKEN;}

      if(globalBHT[globalHistory] >= WT){globalPrediction=TAKEN;}
      else{globalPrediction=NOTTAKEN;}

      uint32_t choiceID = globalHistory;

      if (localPrediction != globalPrediction) {
        if (outcome == localPrediction) {
          if(choiceBHT[choiceID] == SN){choiceBHT[choiceID] = SN;}
          else{choiceBHT[choiceID] = choiceBHT[choiceID] -1;}
        } else if (outcome == globalPrediction) {
          if(choiceBHT[choiceID] == ST){choiceBHT[choiceID] = ST;}
          else{choiceBHT[choiceID] = choiceBHT[choiceID] = +1;}
        }
      }

      localHistoryTable[localID] = ((localHistory << 1) | outcome) & ((1 << lhistoryBits) - 1);
      ghistory = ((ghistory << 1) | outcome) & ((1 << ghistoryBits) - 1);
      break;
    }
    case CUSTOM: {

      uint32_t localID = pc & ((1 << pcIndexBits) - 1);
      uint32_t localHistory = localHistoryTable[localID];
      uint32_t localBHTID = localHistory & ((1 << lhistoryBits) - 1);

      if (outcome == TAKEN) {
        if(localBHT[localBHTID] == ST){localBHT[localBHTID]=ST;}
        else{localBHT[localBHTID]=localBHT[localBHTID] + 1;}
      } 
      else {
        if(localBHT[localBHTID] == SN){localBHT[localBHTID] = SN;}
        else{localBHT[localBHTID] = localBHT[localBHTID] - 1;}
      }

      uint32_t globalHistory = (pc ^ ghistory) & ((1 << ghistoryBits) - 1);

      if (outcome == TAKEN) {
        if(globalBHT[globalHistory] == ST){globalBHT[globalHistory] = ST;}
        else{globalBHT[globalHistory] = globalBHT[globalHistory] + 1;}
      } else {
        if(globalBHT[globalHistory] == SN){globalBHT[globalHistory] = SN;}
        else{globalBHT[globalHistory] = globalBHT[globalHistory] - 1;}
      }

      uint8_t localPrediction;
      uint8_t globalPrediction;

      if(localBHT[localBHTID] >= WT){localPrediction=TAKEN;}
      else{localPrediction=NOTTAKEN;}

      if(globalBHT[globalHistory] >= WT){globalPrediction=TAKEN;}
      else{globalPrediction=NOTTAKEN;}

      uint32_t choiceID = globalHistory;

      if (localPrediction != globalPrediction) {
        if (outcome == localPrediction) {
          if(choiceBHT[choiceID] == SN){choiceBHT[choiceID] = SN;}
          else{choiceBHT[choiceID] = choiceBHT[choiceID] -1;}
        } else if (outcome == globalPrediction) {
          if(choiceBHT[choiceID] == ST){choiceBHT[choiceID] = ST;}
          else{choiceBHT[choiceID] = choiceBHT[choiceID] = +1;}
        }
      }

      localHistoryTable[localID] = ((localHistory << 1) | outcome) & ((1 << lhistoryBits) - 1);
      ghistory = ((ghistory << 1) | outcome) & ((1 << ghistoryBits) - 1);
      break;
    }
  }
}
