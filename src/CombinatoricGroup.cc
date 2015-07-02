/////////////////////////////////////////////////////////////////////////
//   RestFrames: particle physics event analysis library
//   --------------------------------------------------------------------
//   Copyright (c) 2014-2015, Christopher Rogan
/////////////////////////////////////////////////////////////////////////
///
///  \file   CombinatoricGroup.cc
///
///  \author Christopher Rogan
///          (crogan@cern.ch)
///
///  \date   2015 Jan
///
//   This file is part of RestFrames.
//
//   RestFrames is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
// 
//   RestFrames is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
// 
//   You should have received a copy of the GNU General Public License
//   along with RestFrames. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////

#include "RestFrames/CombinatoricGroup.hh"
#include "RestFrames/CombinatoricState.hh"
#include "RestFrames/RFrame.hh"
#include "RestFrames/Jigsaw.hh"

using namespace std;

namespace RestFrames {

  ///////////////////////////////////////////////
  // CombinatoricGroup class
  // a combinatoric collection of particles
  ///////////////////////////////////////////////

  CombinatoricGroup::CombinatoricGroup(const string& sname,const string& stitle) : 
    Group(sname, stitle)
  {
    Init();
  }

  CombinatoricGroup::~CombinatoricGroup(){
    Clear();
  }

  void CombinatoricGroup::Init(){
    m_Type = GCombinatoric;
  }

  void CombinatoricGroup::Clear(){
    ClearElements();
    m_NElementsForFrame.clear();
    m_NExclusiveElementsForFrame.clear(); 
    int Ninit = m_InitStates.GetN();
    for(int i = 0; i < Ninit; i++)
      delete m_InitStates.Get(i);
    m_InitStates.Clear();
  }

  void CombinatoricGroup::AddFrame(RestFrame& frame){
    AddFrame(&frame);
  }

  void CombinatoricGroup::AddFrame(RestFrame* framePtr){
    if(!framePtr->IsRFrame() || !framePtr->IsVisibleFrame()) return;
    RFrame *ptr = dynamic_cast<RFrame*>(framePtr);
    if(ptr){
      ptr->SetGroup(this);
      m_Frames.Add(framePtr);
      if(m_Frames.GetN() == int(m_NElementsForFrame.size())) return;
      m_NElementsForFrame.push_back(1.);
      m_NExclusiveElementsForFrame.push_back(false);
    }
  }

  void CombinatoricGroup::SetNElementsForFrame(const RestFrame& frame, int N, bool exclusive_N){
    SetNElementsForFrame(&frame,N,exclusive_N);
  }
  
  void CombinatoricGroup::SetNElementsForFrame(const RestFrame* framePtr, int N, bool exclusive_N){
    int index = m_Frames.GetIndex(framePtr);
    if(index < 0) return;
    m_NElementsForFrame[index] = N;
    m_NExclusiveElementsForFrame[index] = exclusive_N;
  }

  void CombinatoricGroup::GetNElementsForFrame(const RestFrame* framePtr, int& N, bool& exclusive_N){
    int index = m_Frames.GetIndex(framePtr);
    if(index < 0){
      N = -1;
      exclusive_N = false;
      return;
    }
    N = m_NElementsForFrame[index];
    exclusive_N = m_NExclusiveElementsForFrame[index];
  }

  bool CombinatoricGroup::AddJigsaw(Jigsaw& jigsaw){
    return AddJigsaw(&jigsaw);
  }

  bool CombinatoricGroup::AddJigsaw(Jigsaw* jigsawPtr){
    if(!jigsawPtr) return false;
    if(jigsawPtr->GetGroup()) return false;
    if(!jigsawPtr->IsCombinatoricJigsaw()) return false;
    if(m_JigsawsToUse.Add(jigsawPtr)) jigsawPtr->SetGroup(this);
    return true;
  }

  State* CombinatoricGroup::InitializeGroupState(){
    return new CombinatoricState();
  }

  void CombinatoricGroup::ClearElements(){
    m_StateElements.Clear();
  }

  void CombinatoricGroup::AddElement(State* statePtr){
    m_StateElements.Add(statePtr);
  }

  int CombinatoricGroup::GetNElements() const{
    return m_StateElements.GetN();
  }

  // Event analysis functions
  void CombinatoricGroup::ClearEvent(){
    ClearFourVectors();
  }
 
  bool CombinatoricGroup::AnalyzeEvent(){
    if(!IsSoundMind() || !m_GroupStatePtr){
      m_Log << LogWarning << "Unable to Analyze Event" << m_End;
      SetSpirit(false);
      return false;
    }
    
    CombinatoricState* group_statePtr = dynamic_cast<CombinatoricState*>(m_GroupStatePtr);
    if(!group_statePtr){
      m_Log << LogWarning << "Unable to get Group CombinatoricState" << m_End;
      SetSpirit(false);
      return false;
    }
    
    group_statePtr->ClearElements();
    group_statePtr->AddElement(m_StateElements);    

    SetSpirit(true);
    return true;
  }

  void CombinatoricGroup::ClearFourVectors(){
    ClearElements();
   }

  GroupElementID CombinatoricGroup::AddLabFrameFourVector(const TLorentzVector& V){
    State* statePtr = GetNewState();
    
    TLorentzVector P = V;
    if(P.M() < 0.) P.SetVectM(V.Vect(),0.);
    statePtr->SetFourVector(P);
    AddElement(statePtr);
   
    return statePtr;
  }

  int CombinatoricGroup::GetNFourVectors() const{
    return GetNElements();
  }

  const RestFrame* CombinatoricGroup::GetFrame(const GroupElementID elementID){
    //State* elementPtr = (State*)elementID;
    const State* elementPtr = elementID;
    int N = m_States.GetN();
    for(int i = N-1; i >= 0; i--){
      CombinatoricState* statePtr = dynamic_cast<CombinatoricState*>(m_States.Get(i));
      if(!statePtr) continue;
      if(statePtr->ContainsElement(elementPtr)){
	RestFrame* framePtr = statePtr->GetFrame();
	if(framePtr) return framePtr;
      }
    }
    return nullptr;
  }

  TLorentzVector CombinatoricGroup::GetLabFrameFourVector(const GroupElementID elementID){
    const State* elementPtr = elementID;
    TLorentzVector P(0.,0.,0.,0.);
    int N = m_States.GetN();
    for(int i = N-1; i >= 0; i--){
      CombinatoricState* statePtr = dynamic_cast<CombinatoricState*>(m_States.Get(i));
      if(!statePtr) continue;
      if(statePtr->ContainsElement(elementPtr)){
	P = elementPtr->GetFourVector();
	break;
      }
    }
    return P;
  }

  int CombinatoricGroup::GetNElementsInFrame(const RestFrame& frame){
    return GetNElementsInFrame(&frame);
  }
  int CombinatoricGroup::GetNElementsInFrame(const RestFrame* framePtr){
    if(!framePtr) return -1;
    if(!ContainsFrame(framePtr)) return -1;
    CombinatoricState* statePtr = dynamic_cast<CombinatoricState*>(GetState(framePtr));
    if(!statePtr) return -1;
    return statePtr->GetNElements();
  }

  State* CombinatoricGroup::GetNewState(){
    if(GetNElements() < m_InitStates.GetN())
      return m_InitStates.Get(GetNElements());

    State* statePtr = new State();
    m_InitStates.Add(statePtr);
    return statePtr;
  }

}
