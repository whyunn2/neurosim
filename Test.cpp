/*******************************************************************************
* Copyright (c) 2015-2017
* School of Electrical, Computer and Energy Engineering, Arizona State University
* PI: Prof. Shimeng Yu
* All rights reserved.
*   
* This source code is part of NeuroSim - a device-circuit-algorithm framework to benchmark 
* neuro-inspired architectures with synaptic devices(e.g., SRAM and emerging non-volatile memory). 
* Copyright of the model is maintained by the developers, and the model is distributed under 
* the terms of the Creative Commons Attribution-NonCommercial 4.0 International Public License 
* http://creativecommons.org/licenses/by-nc/4.0/legalcode.
* The source code is free and you can redistribute and/or modify it
* by providing that the following conditions are met:
*   
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer. 
*   
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* Developer list: 
*   Pai-Yu Chen     Email: pchen72 at asu dot edu 
*                     
*   Xiaochen Peng   Email: xpeng15 at asu dot edu
********************************************************************************/

#include <cstdio>
#include <iostream>
#include <vector>
#include <random>
#include "formula.h"
#include "Param.h"
#include "Array.h"
#include "Mapping.h"
#include "NeuroSim.h"
#include "Cell.h"

extern Param *param;

extern std::vector< std::vector<double> > testInput;
extern std::vector< std::vector<int> > dTestInput;
extern std::vector< std::vector<double> > testOutput;
extern std::vector< std::vector<double> > Input;
extern std::vector< std::vector<int> > dInput;
extern std::vector< std::vector<double> > Output;

extern std::vector< std::vector<double> > weight1;
extern std::vector< std::vector<double> > weight2;
extern std::vector< std::vector<double> > weight3;

extern Technology techIH;
extern Technology techHO;
extern Technology techHH;
extern Array *arrayIH;
extern Array *arrayHO;
extern Array *arrayHH;
extern SubArray *subArrayIH;
extern SubArray *subArrayHO;
extern SubArray *subArrayHH;
extern Adder adderIH;
extern Mux muxIH;
extern RowDecoder muxDecoderIH;
extern DFF dffIH;
extern Subtractor subtractorIH;
extern Adder adderHH;
extern Mux muxHH;
extern RowDecoder muxDecoderHH;
extern DFF dffHH;
extern Subtractor subtractorHH;
extern Adder adderHO;
extern Mux muxHO;
extern RowDecoder muxDecoderHO;
extern DFF dffHO;
extern Subtractor subtractorHO;

extern int correct;		// # of correct prediction

/* Validation */
void Validate() {
	int numBatchReadSynapse;    // # of read synapses in a batch read operation (decide later)
	double outN1[param->nHide]; // Net input to the hidden layer [param->nHide]
	double a1[param->nHide];    // Net output of hidden layer [param->nHide] also the input of hidden layer to output layer
	int da1[param->nHide];  // Digitized net output of hidden layer [param->nHide] also the input of hidden layer to output layer
	double outN2[param->nOutput];   // Net input to the output layer [param->nOutput]
	double a2[param->nOutput];  // Net output of output layer [param->nOutput]
	double outN3[param->nHide2]; //// Net input to the second hidden layer
	double a3[param->nHide2]; ////Net output of second hidden layer
	int da3[param->nHide2]; //// Digitized net output of second hidden layer
	double tempMax;
	int countNum;
	correct = 0;

	double sumArrayReadEnergyIH = 0;   // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumNeuroSimReadEnergyIH = 0;   // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumReadLatencyIH = 0;    // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumArrayReadEnergyHH = 0;   // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumNeuroSimReadEnergyHH = 0;   // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumReadLatencyHH = 0;
    double readVoltageIH,readVoltageHO,readVoltageHH,readVoltageMSB;
    double readPulseWidthIH,readPulseWidthHO,readPulseWidthHH,readPulseWidthMSB;
	double sumArrayReadEnergyHO = 0;    // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumNeuroSimReadEnergyHO = 0; // Use a temporary variable here since OpenMP does not support reduction on class member
	double sumReadLatencyHO = 0;    // Use a temporary variable here since OpenMP does not support reduction on class member
    if(eNVM* temp = dynamic_cast<eNVM*>(arrayIH->cell[0][0]))
    {
        readVoltageIH = static_cast<eNVM*>(arrayIH->cell[0][0])->readVoltage;
		readVoltageHH = static_cast<eNVM*>(arrayHH->cell[0][0])->readVoltage;
        readVoltageHO = static_cast<eNVM*>(arrayHO->cell[0][0])->readVoltage;
        readPulseWidthIH = static_cast<eNVM*>(arrayIH->cell[0][0])->readPulseWidth;
		readPulseWidthHH = static_cast<eNVM*>(arrayHH->cell[0][0])->readPulseWidth;
	    readPulseWidthHO = static_cast<eNVM*>(arrayHO->cell[0][0])->readPulseWidth;
    }
    else if(HybridCell* temp = dynamic_cast<HybridCell*>(arrayIH->cell[0][0]))
    {         
        readVoltageIH = static_cast<HybridCell*>(arrayIH->cell[0][0])->LSBcell.readVoltage;
		readVoltageHH = static_cast<HybridCell*>(arrayHH->cell[0][0])->LSBcell.readVoltage;
        readVoltageHO = static_cast<HybridCell*>(arrayHO->cell[0][0])->LSBcell.readVoltage;
        readVoltageMSB = static_cast<HybridCell*>(arrayIH->cell[0][0])->MSBcell_LTP.readVoltage;
        readPulseWidthIH = static_cast<HybridCell*>(arrayIH->cell[0][0])->LSBcell.readPulseWidth;
		readPulseWidthHH = static_cast<HybridCell*>(arrayHH->cell[0][0])->LSBcell.readPulseWidth;
	    readPulseWidthHO = static_cast<HybridCell*>(arrayHO->cell[0][0])->LSBcell.readPulseWidth; 
	    readPulseWidthMSB = static_cast<HybridCell*>(arrayHO->cell[0][0])->MSBcell_LTP.readPulseWidth;       

    }
    
    #pragma omp parallel for private(outN1, a1, da1, outN2, a2, outN3, a3, da3, tempMax, countNum, numBatchReadSynapse) reduction(+: correct, sumArrayReadEnergyIH, sumNeuroSimReadEnergyIH, sumArrayReadEnergyHH, sumNeuroSimReadEnergyHH, sumArrayReadEnergyHO, sumNeuroSimReadEnergyHO, sumReadLatencyIH, sumReadLatencyHH, sumReadLatencyHO)
	for (int i = 0; i < param->numMnistTestImages; i++)
	{
		// Forward propagation
		/* First layer from input layer to the hidden layer */
		std::fill_n(outN1, param->nHide, 0);
		std::fill_n(a1, param->nHide, 0);
		if (param->useHardwareInTestingFF) {    // Hardware
			for (int j=0; j<param->nHide; j++) {
				if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayIH->cell[0][0])) {  // Analog eNVM
					if (static_cast<eNVM*>(arrayIH->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyIH += arrayIH->wireGateCapRow * techIH.vdd * techIH.vdd * param->nInput; // All WLs open
					}
				} else if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayIH->cell[0][0])) { // Digital eNVM
					if (static_cast<eNVM*>(arrayIH->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyIH += arrayIH->wireGateCapRow * techIH.vdd * techIH.vdd;  // Selected WL
					} else {    // Cross-point
						sumArrayReadEnergyIH += arrayIH->wireCapRow * techIH.vdd * techIH.vdd * (param->nInput - 1);    // Unselected WLs
					}
				}else if (HybridCell *temp = dynamic_cast<HybridCell*>(arrayIH->cell[0][0]))  // 3T1C cell
						sumArrayReadEnergyIH += arrayIH->wireGateCapRow * techIH.vdd * techIH.vdd * param->nInput; // All WLs open
				
                for (int n=0; n<param->numBitInput; n++) {
					double pSumMaxAlgorithm = pow(2, n) / (param->numInputLevel - 1) * arrayIH->arrayRowSize;   // Max algorithm partial weighted sum for the nth vector bit (if both max input value and max weight are 1)
					if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayIH->cell[0][0])) {  // Analog eNVM
						double Isum = 0;    // weighted sum current
						double IsumMax = 0; // Max weighted sum current
						double IsumMin = 0; // Max weighted sum current
						double inputSum = 0;    // Weighted sum current of input vector * weight=1 column
						for (int k=0; k<param->nInput; k++) {
							if ((dTestInput[i][k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
								Isum += arrayIH->ReadCell(j,k);
								inputSum += arrayIH->GetMediumCellReadCurrent(j,k);
								sumArrayReadEnergyIH += arrayIH->wireCapRow * readVoltageIH * readVoltageIH;   // Selected BLs (1T1R) or Selected WLs (cross-point)
							}
							IsumMax += arrayIH->GetMaxCellReadCurrent(j,k);
							IsumMin += arrayIH->GetMinCellReadCurrent(j,k);
						}
						sumArrayReadEnergyIH += Isum * readVoltageIH * readPulseWidthIH;
						int outputDigits = (CurrentToDigits(Isum, IsumMax-IsumMin)-CurrentToDigits(inputSum, IsumMax-IsumMin));
                        //int outputDigits = (CurrentToDigits(Isum, IsumMax)-CurrentToDigits(inputSum, IsumMax));
						outN1[j] += DigitsToAlgorithm(outputDigits, pSumMaxAlgorithm);
					} 
                    else if(HybridCell* temp = dynamic_cast<HybridCell*>(arrayIH->cell[0][0]))
                    {
                        double Isum_LSB = 0;              // weighted sum current of the LTP cell
                        double Isum_MSB_LTP = 0;    // weighted sum current of the LTP cell
                        double Isum_MSB_LTD = 0;    // weighted sum current of the LTP cell
                        double IsumMax_LSB = 0;            //the maximum weight sum current (all cells are at high conductance)
                        double IsumMin_LSB = 0;
                        double IsumMax_MSB = 0;
                        double IsumMin_MSB = 0;                        
                        double inputSum_LSB= 0;      // Reference for LSB cell
                        for (int k=0; k<param->nInput; k++) {
							if ((dTestInput[i][k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
								Isum_LSB += arrayIH->ReadCell(j,k,"LSB");
                                Isum_MSB_LTP += arrayIH->ReadCell(j,k,"MSB_LTP");  
                                Isum_MSB_LTD += arrayIH->ReadCell(j,k,"MSB_LTD");  
								inputSum_LSB += arrayIH->GetMediumCellReadCurrent(j,k);
                                sumArrayReadEnergyIH += arrayIH->wireCapRow * readVoltageIH * readVoltageIH;   // Selected BLs (1T1R) or Selected WLs (cross-point)
                                sumArrayReadEnergyIH += 2*arrayIH->wireCapRow * readVoltageMSB * readVoltageMSB; // Selected BLs (1T1R) or Selected WLs (cross-point)
							}
							IsumMax_LSB += arrayIH->GetMaxCellReadCurrent(j,k,"LSB");
                         	IsumMin_LSB += arrayIH->GetMinCellReadCurrent(j,k,"LSB");
                            IsumMax_MSB += arrayIH->GetMaxCellReadCurrent(j,k,"MSB");
                            IsumMin_MSB += arrayIH->GetMinCellReadCurrent(j,k,"MSB");
						}
                        sumArrayReadEnergyIH += Isum_LSB * readVoltageIH * readPulseWidthIH;
                        sumArrayReadEnergyIH += (Isum_MSB_LTP + Isum_MSB_LTD) * readVoltageMSB * readPulseWidthMSB;
                        int outputDigits;
                        int outputDigitsLSB = 2*(CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(inputSum_LSB, IsumMax_LSB-IsumMin_LSB)); //minus the reference
                        //int outputDigitsLSB = CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(inputSum_LSB, IsumMax_LSB-IsumMin_LSB); //minus the reference
                        int outputDigitsMSB = CurrentToDigits(Isum_MSB_LTP, IsumMax_MSB-IsumMin_MSB)-CurrentToDigits(Isum_MSB_LTD, IsumMax_MSB-IsumMin_MSB); //minus the reference
                        outputDigits = static_cast<HybridCell*>(arrayIH->cell[0][0])->significance*outputDigitsMSB+outputDigitsLSB;
                        outN1[j] += DigitsToAlgorithm(outputDigits/3, pSumMaxAlgorithm)/(static_cast<HybridCell*>(arrayIH->cell[0][0])->significance+1);;   
                    }
                    else {
                            bool digitalNVM = false; 
                            bool parallelRead = false;
                            if(DigitalNVM*temp = dynamic_cast<DigitalNVM*>(arrayIH->cell[0][0]))
                            {    digitalNVM = true;
                                if(static_cast<DigitalNVM*>(arrayIH->cell[0][0])->parallelRead == true) 
								{
                                    parallelRead = true;
                                }
                            }
                            if(digitalNVM && parallelRead) // parallel read-out for DigitalNVM
                            {
                                    double Imax = static_cast<DigitalNVM*>(arrayIH->cell[0][0])->avgMaxConductance*static_cast<DigitalNVM*>(arrayIH->cell[0][0])->readVoltage;
                                    double Imin = static_cast<DigitalNVM*>(arrayIH->cell[0][0])->avgMinConductance*static_cast<DigitalNVM*>(arrayIH->cell[0][0])->readVoltage;
                                    double Isum = 0;    // weighted sum current
							        double IsumMax = 0; // Max weighted sum current
							        double inputSum = 0;    // Weighted sum current of input vector * weight=1 column
                                    int Dsum=0;
                                    int DsumMax = 0;
                                    int Dref = 0;
                                    for (int w=0;w<param->numWeightBit;w++){
                                        int colIndex = (j+1) * param->numWeightBit - (w+1);  // w=0 is the LSB
									    for (int k=0; k<param->nInput; k++) 
                                        {
										    if((dTestInput[i][k]>>n) & 1){ // accumulate the current along a column
											    Isum += static_cast<DigitalNVM*>(arrayIH->cell[colIndex ][k])->conductance*static_cast<DigitalNVM*>(arrayIH->cell[colIndex ][k])->readVoltage;
											    //inputSum += Imin;
                                                inputSum += static_cast<DigitalNVM*>(arrayIH->cell[arrayIH->refColumnNumber][k])->conductance*static_cast<DigitalNVM*>(arrayIH->cell[arrayIH->refColumnNumber][k])->readVoltage;
										    }
									    }
                                       /* int outputDigits = (Isum - inputSum)/(Imax-Imin); // the output at the ADC of this column
                                                                                                               // basically, this is the number of "1" in this column
                                        if(outputDigits > param->pSumMaxHardware)
                                            outputDigits = param->pSumMaxHardware; */
                                        int outputDigits = (int) (Isum /(Imax-Imin)); // the output at the ADC of this column
                                                                                                               // basically, this is the number of "1" in this column
                                        int outputDigitsRef = (int) (inputSum/(Imax-Imin));
                                    
                                        if(outputDigits > param->pSumMaxHardware)
                                            outputDigits = param->pSumMaxHardware;
                                        if(outputDigitsRef > param->pSumMaxHardware)
                                            outputDigitsRef = param->pSumMaxHardware;
                                        outputDigits = outputDigits-outputDigitsRef;
                                            
                                        Dref = (int)(inputSum/Imin);
                                        Isum=0;
                                        inputSum=0;
                                        Dsum += outputDigits*(int) pow(2,w);  // get the weight represented by the column
                                        DsumMax += param->nInput*(int) pow(2,w); // the maximum weight that can be represented by this column
        
                                    }
                                    outN1[j] += (double)(Dsum - Dref*(pow(2,param->numWeightBit-1)-1)) / DsumMax * pSumMaxAlgorithm;
                                    sumArrayReadEnergyIH  += static_cast<DigitalNVM*>(arrayIH->cell[0][0])->readEnergy * arrayIH->numCellPerSynapse * arrayIH->arrayRowSize;
                            }
                            else
                            {	 // Digital NVM or SRAM row-by-row readout				
							    int Dsum = 0;
							    int DsumMax = 0;
							    int inputSum = 0;
							    for (int k=0; k<param->nInput; k++) {
								    if ((dTestInput[i][k]>>n) & 1) {    // if the nth bit of dInput[i][k] is 1
									    Dsum += (int)(arrayIH->ReadCell(j,k));
									    inputSum += pow(2, arrayIH->numCellPerSynapse-1) - 1;   // get the digital weights of the dummy column as reference
								    }
								    DsumMax += pow(2, arrayIH->numCellPerSynapse) - 1;
							    }
							    if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayIH->cell[0][0])) {    // Digital eNVM
								    sumArrayReadEnergyIH  += static_cast<DigitalNVM*>(arrayIH->cell[0][0])->readEnergy * arrayIH->numCellPerSynapse * arrayIH->arrayRowSize;
							    } 
                                else {    // SRAM
								    sumArrayReadEnergyIH  += static_cast<SRAM*>(arrayIH->cell[0][0])->readEnergy * arrayIH->numCellPerSynapse * arrayIH->arrayRowSize;
							    }
							    outN1[j] += (double)(Dsum - inputSum) / DsumMax * pSumMaxAlgorithm;
							}
                    }
                }
				a1[j] = sigmoid(outN1[j]);
				//da1[j] = round(a1[j] * (param->numInputLevel - 1));
				da1[j] = round_th(a1[j]*(param->numInputLevel-1), param->Hthreshold);
			}

			numBatchReadSynapse = (int)ceil((double)param->nHide/param->numColMuxed);
			#pragma omp critical    // Use critical here since NeuroSim class functions may update its member variables
			for (int j=0; j<param->nHide; j+=numBatchReadSynapse) {
				int numActiveRows = 0;  // Number of selected rows for NeuroSim
				for (int n=0; n<param->numBitInput; n++) {
					for (int k=0; k<param->nInput; k++) {
						if ((dTestInput[i][k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
							numActiveRows++;
						}
					}
				}
				subArrayIH->activityRowRead = (double)numActiveRows/param->nInput/param->numBitInput;
				sumNeuroSimReadEnergyIH += NeuroSimSubArrayReadEnergy(subArrayIH);
				sumNeuroSimReadEnergyIH += NeuroSimNeuronReadEnergy(subArrayIH, adderIH, muxIH, muxDecoderIH, dffIH, subtractorIH);
				sumReadLatencyIH += NeuroSimSubArrayReadLatency(subArrayIH);
				sumReadLatencyIH += NeuroSimNeuronReadLatency(subArrayIH, adderIH, muxIH, muxDecoderIH, dffIH, subtractorIH);
			}
		} else {    // Algorithm
			for (int j=0; j<param->nHide; j++){
				for (int k=0; k<param->nInput; k++){
					outN1[j] += testInput[i][k] * weight1[j][k];
				}
				a1[j] = sigmoid(outN1[j]);
			}
		}

		/* Second layer from hidden layer to the second hidden layer */
		std::fill_n(outN3, param->nHide2, 0);
		std::fill_n(a3, param->nHide2, 0);
		if (param->useHardwareInTestingFF) {    // Hardware
			for (int j=0; j<param->nHide2; j++) {
				if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayHH->cell[0][0])) {  // Analog eNVM
					if (static_cast<eNVM*>(arrayHH->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyHH += arrayHH->wireGateCapRow * techHH.vdd * techHH.vdd * param->nHide; // All WLs open
					}
				} else if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayHH->cell[0][0])) { // Digital eNVM
					if (static_cast<eNVM*>(arrayHH->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyHH += arrayHH->wireGateCapRow * techHH.vdd * techHH.vdd;  // Selected WL
					} else {    // Cross-point
						sumArrayReadEnergyHH += arrayHH->wireCapRow * techHH.vdd * techHH.vdd * (param->nHide - 1);    // Unselected WLs
					}
				}else if (HybridCell *temp = dynamic_cast<HybridCell*>(arrayHH->cell[0][0]))  // 3T1C cell
						sumArrayReadEnergyHH += arrayHH->wireGateCapRow * techHH.vdd * techHH.vdd * param->nHide; // All WLs open
				
                for (int n=0; n<param->numBitInput; n++) {
					double pSumMaxAlgorithm = pow(2, n) / (param->numInputLevel - 1) * arrayHH->arrayRowSize;   // Max algorithm partial weighted sum for the nth vector bit (if both max input value and max weight are 1)
					if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayHH->cell[0][0])) {  // Analog eNVM
						double Isum = 0;    // weighted sum current
						double IsumMax = 0; // Max weighted sum current
						double IsumMin = 0; // Max weighted sum current
						double a2Sum = 0;    // Weighted sum current of input vector * weight=1 column
						for (int k=0; k<param->nHide; k++) {
							if ((da1[k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
								Isum += arrayHH->ReadCell(j,k);
								a2Sum += arrayHH->GetMediumCellReadCurrent(j,k);
								sumArrayReadEnergyHH += arrayHH->wireCapRow * readVoltageHH * readVoltageHH;   // Selected BLs (1T1R) or Selected WLs (cross-point)
							}
							IsumMax += arrayHH->GetMaxCellReadCurrent(j,k);
							IsumMin += arrayHH->GetMinCellReadCurrent(j,k);
						}
						sumArrayReadEnergyHH += Isum * readVoltageHH * readPulseWidthHH;
						int outputDigits = (CurrentToDigits(Isum, IsumMax-IsumMin)-CurrentToDigits(a2Sum, IsumMax-IsumMin));
                        //int outputDigits = (CurrentToDigits(Isum, IsumMax)-CurrentToDigits(inputSum, IsumMax));
						outN3[j] += DigitsToAlgorithm(outputDigits, pSumMaxAlgorithm);
					} 
                    else if(HybridCell* temp = dynamic_cast<HybridCell*>(arrayHH->cell[0][0]))
                    {
                        double Isum_LSB = 0;              // weighted sum current of the LTP cell
                        double Isum_MSB_LTP = 0;    // weighted sum current of the LTP cell
                        double Isum_MSB_LTD = 0;    // weighted sum current of the LTP cell
                        double IsumMax_LSB = 0;            //the maximum weight sum current (all cells are at high conductance)
                        double IsumMin_LSB = 0;
                        double IsumMax_MSB = 0;
                        double IsumMin_MSB = 0;                        
                        double a2Sum_LSB= 0;      // Reference for LSB cell
                        for (int k=0; k<param->nHide; k++) {
							if ((da1[k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
								Isum_LSB += arrayHH->ReadCell(j,k,"LSB");
                                Isum_MSB_LTP += arrayHH->ReadCell(j,k,"MSB_LTP");  
                                Isum_MSB_LTD += arrayHH->ReadCell(j,k,"MSB_LTD");  
								a2Sum_LSB += arrayHH->GetMediumCellReadCurrent(j,k);
                                sumArrayReadEnergyIH += arrayHH->wireCapRow * readVoltageIH * readVoltageIH;   // Selected BLs (1T1R) or Selected WLs (cross-point)
                                sumArrayReadEnergyIH += 2*arrayHH->wireCapRow * readVoltageMSB * readVoltageMSB; // Selected BLs (1T1R) or Selected WLs (cross-point)
							}
							IsumMax_LSB += arrayHH->GetMaxCellReadCurrent(j,k,"LSB");
                         	IsumMin_LSB += arrayHH->GetMinCellReadCurrent(j,k,"LSB");
                            IsumMax_MSB += arrayHH->GetMaxCellReadCurrent(j,k,"MSB");
                            IsumMin_MSB += arrayHH->GetMinCellReadCurrent(j,k,"MSB");
						}
                        sumArrayReadEnergyHH += Isum_LSB * readVoltageHH * readPulseWidthHH;
                        sumArrayReadEnergyHH += (Isum_MSB_LTP + Isum_MSB_LTD) * readVoltageMSB * readPulseWidthMSB;
                        int outputDigits;
                        int outputDigitsLSB = 2*(CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(a2Sum_LSB, IsumMax_LSB-IsumMin_LSB)); //minus the reference
                        //int outputDigitsLSB = CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(inputSum_LSB, IsumMax_LSB-IsumMin_LSB); //minus the reference
                        int outputDigitsMSB = CurrentToDigits(Isum_MSB_LTP, IsumMax_MSB-IsumMin_MSB)-CurrentToDigits(Isum_MSB_LTD, IsumMax_MSB-IsumMin_MSB); //minus the reference
                        outputDigits = static_cast<HybridCell*>(arrayHH->cell[0][0])->significance*outputDigitsMSB+outputDigitsLSB;
                        outN3[j] += DigitsToAlgorithm(outputDigits/3, pSumMaxAlgorithm)/(static_cast<HybridCell*>(arrayHH->cell[0][0])->significance+1);;   
                    }
                    else {
                            bool digitalNVM = false; 
                            bool parallelRead = false;
                            if(DigitalNVM*temp = dynamic_cast<DigitalNVM*>(arrayHH->cell[0][0]))
                            {    digitalNVM = true;
                                if(static_cast<DigitalNVM*>(arrayHH->cell[0][0])->parallelRead == true) 
								{
                                    parallelRead = true;
                                }
                            }
                            if(digitalNVM && parallelRead) // parallel read-out for DigitalNVM
                            {
                                    double Imax = static_cast<DigitalNVM*>(arrayHH->cell[0][0])->avgMaxConductance*static_cast<DigitalNVM*>(arrayHH->cell[0][0])->readVoltage;
                                    double Imin = static_cast<DigitalNVM*>(arrayHH->cell[0][0])->avgMinConductance*static_cast<DigitalNVM*>(arrayHH->cell[0][0])->readVoltage;
                                    double Isum = 0;    // weighted sum current
							        double IsumMax = 0; // Max weighted sum current
							        double inputSum = 0;    // Weighted sum current of input vector * weight=1 column
                                    int Dsum=0;
                                    int DsumMax = 0;
                                    int Dref = 0;
                                    for (int w=0;w<param->numWeightBit;w++){
                                        int colIndex = (j+1) * param->numWeightBit - (w+1);  // w=0 is the LSB
									    for (int k=0; k<param->nInput; k++) 
                                        {
										    if((da1[k]>>n) & 1){ // accumulate the current along a column
											    Isum += static_cast<DigitalNVM*>(arrayHH->cell[colIndex ][k])->conductance*static_cast<DigitalNVM*>(arrayHH->cell[colIndex ][k])->readVoltage;
											    //inputSum += Imin;
                                                inputSum += static_cast<DigitalNVM*>(arrayHH->cell[arrayHH->refColumnNumber][k])->conductance*static_cast<DigitalNVM*>(arrayHH->cell[arrayHH->refColumnNumber][k])->readVoltage;
										    }
									    }
                                       /* int outputDigits = (Isum - inputSum)/(Imax-Imin); // the output at the ADC of this column
                                                                                                               // basically, this is the number of "1" in this column
                                        if(outputDigits > param->pSumMaxHardware)
                                            outputDigits = param->pSumMaxHardware; */
                                        int outputDigits = (int) (Isum /(Imax-Imin)); // the output at the ADC of this column
                                                                                                               // basically, this is the number of "1" in this column
                                        int outputDigitsRef = (int) (inputSum/(Imax-Imin));
                                    
                                        if(outputDigits > param->pSumMaxHardware)
                                            outputDigits = param->pSumMaxHardware;
                                        if(outputDigitsRef > param->pSumMaxHardware)
                                            outputDigitsRef = param->pSumMaxHardware;
                                        outputDigits = outputDigits-outputDigitsRef;
                                            
                                        Dref = (int)(inputSum/Imin);
                                        Isum=0;
                                        inputSum=0;
                                        Dsum += outputDigits*(int) pow(2,w);  // get the weight represented by the column
                                        DsumMax += param->nHide*(int) pow(2,w); // the maximum weight that can be represented by this column
        
                                    }
                                    outN3[j] += (double)(Dsum - Dref*(pow(2,param->numWeightBit-1)-1)) / DsumMax * pSumMaxAlgorithm;
                                    sumArrayReadEnergyHH  += static_cast<DigitalNVM*>(arrayHH->cell[0][0])->readEnergy * arrayHH->numCellPerSynapse * arrayHH->arrayRowSize;
                            }
                            else
                            {	 // Digital NVM or SRAM row-by-row readout				
							    int Dsum = 0;
							    int DsumMax = 0;
							    int a2Sum = 0;
							    for (int k=0; k<param->nHide; k++) {
								    if ((da1[k]>>n) & 1) {    // if the nth bit of dInput[i][k] is 1
									    Dsum += (int)(arrayHH->ReadCell(j,k));
									    a2Sum += pow(2, arrayHH->numCellPerSynapse-1) - 1;   // get the digital weights of the dummy column as reference
								    }
								    DsumMax += pow(2, arrayHH->numCellPerSynapse) - 1;
							    }
							    if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayHH->cell[0][0])) {    // Digital eNVM
								    sumArrayReadEnergyHH  += static_cast<DigitalNVM*>(arrayHH->cell[0][0])->readEnergy * arrayHH->numCellPerSynapse * arrayHH->arrayRowSize;
							    } 
                                else {    // SRAM
								    sumArrayReadEnergyHH  += static_cast<SRAM*>(arrayHH->cell[0][0])->readEnergy * arrayHH->numCellPerSynapse * arrayHH->arrayRowSize;
							    }
							    outN3[j] += (double)(Dsum - a2Sum) / DsumMax * pSumMaxAlgorithm;
							}
                    }
                }
				a3[j] = sigmoid(outN3[j]);
				//da1[j] = round(a1[j] * (param->numInputLevel - 1));
				da3[j] = round_th(a3[j]*(param->numInputLevel-1), param->Hthreshold);
			}

			numBatchReadSynapse = (int)ceil((double)param->nHide2/param->numColMuxed);
			#pragma omp critical    // Use critical here since NeuroSim class functions may update its member variables
			for (int j=0; j<param->nHide2; j+=numBatchReadSynapse) {
				int numActiveRows = 0;  // Number of selected rows for NeuroSim
				for (int n=0; n<param->numBitInput; n++) {
					for (int k=0; k<param->nHide; k++) {
						if ((da1[k]>>n) & 1) {    // if the nth bit of dTestInput[i][k] is 1
							numActiveRows++;
						}
					}
				}
				subArrayHH->activityRowRead = (double)numActiveRows/param->nHide/param->numBitInput;
				sumNeuroSimReadEnergyHH += NeuroSimSubArrayReadEnergy(subArrayHH);
				sumNeuroSimReadEnergyHH += NeuroSimNeuronReadEnergy(subArrayHH, adderHH, muxHH, muxDecoderHH, dffHH, subtractorHH);
				sumReadLatencyHH += NeuroSimSubArrayReadLatency(subArrayHH);
				sumReadLatencyHH += NeuroSimNeuronReadLatency(subArrayHH, adderHH, muxHH, muxDecoderHH, dffHH, subtractorHH);
			}
		} else {    // Algorithm
			for (int j=0; j<param->nHide2; j++){
				for (int k=0; k<param->nHide; k++){
					outN3[j] += a1[k] * weight3[j][k];
				}
				a3[j] = sigmoid(outN3[j]);
			}
		}

		/* Second layer from hidden layer to the output layer */
		tempMax = 0;
		countNum = 0;
		std::fill_n(outN2, param->nOutput, 0);
		std::fill_n(a2, param->nOutput, 0);
		if (param->useHardwareInTestingFF) {  // Hardware
			for (int j=0; j<param->nOutput; j++) {
				if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayHO->cell[0][0])) {  // Analog eNVM
					if (static_cast<eNVM*>(arrayHO->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyHO += arrayHO->wireGateCapRow * techHO.vdd * techHO.vdd * param->nHide; // All WLs open
					}
				} else if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayHO->cell[0][0])) {
					if (static_cast<eNVM*>(arrayHO->cell[0][0])->cmosAccess) {  // 1T1R
						sumArrayReadEnergyHO += arrayHO->wireGateCapRow * techHO.vdd * techHO.vdd;  // Selected WL
					} else {    // Cross-point
						sumArrayReadEnergyHO += arrayHO->wireCapRow * techHO.vdd * techHO.vdd * (param->nHide - 1); // Unselected WLs
					}
				}else if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayHO->cell[0][0]))  // Analog eNVM
						sumArrayReadEnergyHO += arrayHO->wireGateCapRow * techHO.vdd * techHO.vdd * param->nHide; // All WLs open

				for (int n=0; n<param->numBitInput; n++) {
					double pSumMaxAlgorithm = pow(2, n) / (param->numInputLevel - 1) * arrayHO->arrayRowSize;    // Max algorithm partial weighted sum for the nth vector bit (if both max input value and max weight are 1)
					if (AnalogNVM *temp = dynamic_cast<AnalogNVM*>(arrayHO->cell[0][0])) {  // Analog NVM
						double Isum = 0;    // weighted sum current
						double IsumMax = 0; // Max weighted sum current
                        double IsumMin = 0;
						double a1Sum = 0;   // Weighted sum current of a1 vector * weight=1 column
						for (int k=0; k<param->nHide2; k++) {
							if ((da3[k]>>n) & 1) {    // if the nth bit of da1[k] is 1
								Isum += arrayHO->ReadCell(j,k);
								a1Sum += arrayHO->GetMediumCellReadCurrent(j,k);
								sumArrayReadEnergyHO += arrayHO->wireCapRow * readVoltageHO * readVoltageHO;  
							}
							IsumMax += arrayHO->GetMaxCellReadCurrent(j,k);
                            IsumMin += arrayHO->GetMinCellReadCurrent(j,k);
						}
						sumArrayReadEnergyHO += Isum * readVoltageHO * readPulseWidthHO;
						int outputDigits = (CurrentToDigits(Isum, IsumMax-IsumMin)-CurrentToDigits(a1Sum, IsumMax-IsumMin));
						//int outputDigits = (CurrentToDigits(Isum, IsumMax)-CurrentToDigits(a1Sum, IsumMax));
						outN2[j] += DigitsToAlgorithm(outputDigits, pSumMaxAlgorithm);
                        
					} else if(HybridCell *temp = dynamic_cast<HybridCell*>(arrayHO->cell[0][0])) {  //3T1C
                       
                        double Isum_LSB = 0;              // weighted sum current of the LTP cell
                        double Isum_MSB_LTP = 0;    // weighted sum current of the LTP cell
                        double Isum_MSB_LTD = 0;    // weighted sum current of the LTP cell
                        double IsumMax_LSB = 0;            //the maximum weight sum current (all cells are at high conductance)
                        double IsumMin_LSB = 0;
                        double IsumMax_MSB = 0; 
                        double IsumMin_MSB = 0;                         
                        double a1Sum_LSB= 0;      // Reference for LSB cell
						for (int k=0; k<param->nHide2; k++) {
							if ((da3[k]>>n) & 1) {    // if the nth bit of da1[k] is 1
                                Isum_LSB += arrayHO->ReadCell(j,k,"LSB");                   // the weight sum of the Jth column
                                Isum_MSB_LTP += arrayHO->ReadCell(j,k,"MSB_LTP");  
                                Isum_MSB_LTD += arrayHO->ReadCell(j,k,"MSB_LTD");  
                                a1Sum_LSB += arrayHO->GetMediumCellReadCurrent(j,k);
                                sumArrayReadEnergyHO += arrayHO->wireCapRow * readVoltageHO * readVoltageHO; // Selected BLs (1T1R) or Selected WLs (cross-point)
                                sumArrayReadEnergyHO += 2*arrayHO->wireCapRow * readVoltageMSB * readVoltageMSB; // Selected BLs (1T1R) or Selected WLs (cross-point)
							}
                            IsumMax_LSB += arrayHO->GetMaxCellReadCurrent(j,k,"LSB");
                            IsumMax_MSB += arrayHO->GetMaxCellReadCurrent(j,k,"MSB");
                            IsumMin_LSB += arrayHO->GetMinCellReadCurrent(j,k,"LSB");
                            IsumMin_MSB += arrayHO->GetMinCellReadCurrent(j,k,"MSB");
						}                        
                        sumArrayReadEnergyHO += Isum_LSB * readVoltageHO * readPulseWidthHO;
                        sumArrayReadEnergyHO += (Isum_MSB_LTP + Isum_MSB_LTD) * readVoltageMSB * readPulseWidthMSB;
                        int outputDigits;
                        int outputDigitsLSB = 2*(CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(a1Sum_LSB, IsumMax_LSB-IsumMin_LSB)); //minus the reference
                        //int outputDigitsLSB = CurrentToDigits(Isum_LSB, IsumMax_LSB-IsumMin_LSB)-CurrentToDigits(a1Sum_LSB, IsumMax_LSB-IsumMin_LSB); //minus the reference
                        int outputDigitsMSB = CurrentToDigits(Isum_MSB_LTP, IsumMax_MSB-IsumMin_MSB)-CurrentToDigits(Isum_MSB_LTD, IsumMax_MSB-IsumMin_MSB); //minus the reference
                        outputDigits = static_cast<HybridCell*>(arrayHO->cell[0][0])->significance*outputDigitsMSB+outputDigitsLSB;
                        outN2[j] += DigitsToAlgorithm(outputDigits, pSumMaxAlgorithm)/(static_cast<HybridCell*>(arrayIH->cell[0][0])->significance+1);; 
					} 
                    else 
                        {// SRAM or digital eNVM
                            bool digitalNVM = false; 
                            bool parallelRead = false;
                            if(DigitalNVM*temp = dynamic_cast<DigitalNVM*>(arrayHO->cell[0][0]))
                            {    digitalNVM = true;
                                if(static_cast<DigitalNVM*>(arrayHO->cell[0][0])->parallelRead == true) 
								{
                                    parallelRead = true;
                                }
                            }
                            if(digitalNVM && parallelRead)
                            {
                                //printf("Calculating the weight for parallel read-out\n");
                                double Imin = static_cast<DigitalNVM*>(arrayHO->cell[0][0])->avgMinConductance*static_cast<DigitalNVM*>(arrayHO->cell[0][0])->readVoltage;
                                double Imax = static_cast<DigitalNVM*>(arrayHO->cell[0][0])->avgMaxConductance*static_cast<DigitalNVM*>(arrayHO->cell[0][0])->readVoltage;
                                double Isum = 0;    // weighted sum current
                                double IsumMax = 0; // Max weighted sum current
                                double inputSum = 0;    // Weighted sum current of input vector * weight=1 column
                                int Dsum=0;
                                int DsumMax = 0;
                                int Dref = 0;
                                for (int w=0;w<param->numWeightBit;w++){
                                    int colIndex = (j+1) * param->numWeightBit - (w+1);  // w=0 is the LSB
                                    for (int k=0; k<param->nHide2; k++) {
                                        if ((da3[k]>>n) & 1) { // accumulate the current along a column
                                            Isum += static_cast<DigitalNVM*>(arrayHO->cell[colIndex][k])->conductance*static_cast<DigitalNVM*>(arrayHO->cell[colIndex][k])->readVoltage;
                                            //inputSum += Imin;
                                            inputSum += static_cast<DigitalNVM*>(arrayHO->cell[arrayHO->refColumnNumber][k])->conductance*static_cast<DigitalNVM*>(arrayHO->cell[arrayHO->refColumnNumber][k])->readVoltage;                                            
                                        }
                                    }
                                    int outputDigits = (int) (Isum /(Imax-Imin)); // the output at the ADC of this column
                                                                                                               // basically, this is the number of "1" in this column
                                    int outputDigitsRef = (int) (inputSum/(Imax-Imin));
                                    
                                    if(outputDigits > param->pSumMaxHardware)
                                        outputDigits = param->pSumMaxHardware;
                                    if(outputDigitsRef > param->pSumMaxHardware)
                                        outputDigitsRef = param->pSumMaxHardware;
                                    outputDigits = outputDigits-outputDigitsRef;
 
                                    Dref = (int)(inputSum/Imin);
                                    Isum=0;
                                    inputSum=0;
                                    Dsum += outputDigits*(int) pow(2,w);  // get the weight represented by the column
                                    DsumMax += param->nHide2*(int) pow(2,w); // the maximum weight that can be represented by this column                                        
                                }
                                sumArrayReadEnergyHO += static_cast<DigitalNVM*>(arrayHO->cell[0][0])->readEnergy * arrayHO->numCellPerSynapse * arrayHO->arrayRowSize;
                                outN2[j] += (double)(Dsum - Dref*(pow(2,param->numWeightBit-1)-1)) / DsumMax * pSumMaxAlgorithm;
                            }
                            else
                            {                            
							    int Dsum = 0;
							    int DsumMax = 0;
							    int a1Sum = 0;
							    for (int k=0; k<param->nHide2; k++) {
								    if ((da3[k]>>n) & 1) {    // if the nth bit of da1[k] is 1
									    Dsum += (int)(arrayHO->ReadCell(j,k));
									    a1Sum += pow(2, arrayHO->numCellPerSynapse-1) - 1;    // get current of Dummy Column as reference
								    }
								    DsumMax += pow(2, arrayHO->numCellPerSynapse) - 1;
							    } 
							    if (DigitalNVM *temp = dynamic_cast<DigitalNVM*>(arrayHO->cell[0][0])) {    // Digital eNVM
								    sumArrayReadEnergyHO += static_cast<DigitalNVM*>(arrayHO->cell[0][0])->readEnergy * arrayHO->numCellPerSynapse * arrayHO->arrayRowSize;
							    } 
                                else {
								    sumArrayReadEnergyHO += static_cast<SRAM*>(arrayHO->cell[0][0])->readEnergy * arrayHO->numCellPerSynapse * arrayHO->arrayRowSize;
							    }
							    outN2[j] += (double)(Dsum - a1Sum) / DsumMax * pSumMaxAlgorithm;
                            }
						} 
				}
				a2[j] = sigmoid(outN2[j]);
				if (a2[j] > tempMax) {
					tempMax = a2[j];
					countNum = j;
				}
			}

			numBatchReadSynapse = (int)ceil((double)param->nOutput/param->numColMuxed);
			#pragma omp critical    // Use critical here since NeuroSim class functions may update its member variables
			for (int j=0; j<param->nOutput; j+=numBatchReadSynapse) {
				int numActiveRows = 0;  // Number of selected rows for NeuroSim
				for (int n=0; n<param->numBitInput; n++) {
					for (int k=0; k<param->nHide2; k++) {
						if ((da3[k]>>n) & 1) {    // if the nth bit of da1[k] is 1
							numActiveRows++;
						}
					}
				}
				subArrayHO->activityRowRead = (double)numActiveRows/param->nHide2/param->numBitInput;
				sumNeuroSimReadEnergyHO += NeuroSimSubArrayReadEnergy(subArrayHO);
				sumNeuroSimReadEnergyHO += NeuroSimNeuronReadEnergy(subArrayHO, adderHO, muxHO, muxDecoderHO, dffHO, subtractorHO);
				sumReadLatencyHO += NeuroSimSubArrayReadLatency(subArrayHO);
				sumReadLatencyHO += NeuroSimNeuronReadLatency(subArrayHO, adderHO, muxHO, muxDecoderHO, dffHO, subtractorHO);
			}
		} else {    // Algorithm
			for (int j=0; j<param->nOutput; j++) {
				for (int k=0; k<param->nHide2; k++) {
					outN2[j] += a3[k] * weight2[j][k];
				}
				a2[j] = sigmoid(outN2[j]);
				if (a2[j] > tempMax) {
					tempMax = a2[j];
					countNum = j;
				}
			}
		}
		if (testOutput[i][countNum] == 1) {
			correct++;
		}
	}
	if (!param->useHardwareInTraining) {    // Calculate the classification latency and energy only for offline classification
		arrayIH->readEnergy += sumArrayReadEnergyIH;
		subArrayIH->readDynamicEnergy += sumNeuroSimReadEnergyIH;
		arrayHH->readEnergy += sumArrayReadEnergyHH;
		subArrayHH->readDynamicEnergy += sumNeuroSimReadEnergyHH;
		arrayHO->readEnergy += sumArrayReadEnergyHO;
		subArrayHO->readDynamicEnergy += sumNeuroSimReadEnergyHO;
		subArrayIH->readLatency += sumReadLatencyIH;
		subArrayHH->readLatency += sumReadLatencyHH;
		subArrayHO->readLatency += sumReadLatencyHO;
	}
}

