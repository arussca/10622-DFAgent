// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

# Module definition:
NAME=DFAgent
TYPE=device

# Module content definition:
l:DFAgent.l:vx solgnu linux:y
t:testDFAgent.p:vx solgnu linux:n
e:runDFAgent.p:vx solgnu linux:n
e:useDFAgent.p:vx solgnu linux:n
e:runObservationStrategy.p:linux:n
t:DFObservationStrategyUTests.p:linux:y
t:FITSHeaderInfoUTests.p:linux:y
t:MonitorKeywordUTests.p:linux:y
t:DFAgentUTests.p:linux:y
