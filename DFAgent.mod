// Copyright (c) 2004 Gran Telescopio Canarias, S.A. (GRANTECAN)
// All Rights Reserved

// $Log$

# Module definition:
NAME=DFAgent
TYPE=device

# Module content definition:
l:DFAgent.l:vx solgnu linux:y
t:testDFAgent.p:vx solgnu linux:y
e:runDFAgent.p:vx solgnu linux:y
e:useDFAgent.p:vx solgnu linux:y
e:runObservationStrategy.p:linux:y
t:DFObservationStrategyUTests.p:linux:y
t:FITSHeaderInfoUTests.p:linux:y
t:MonitorKeywordUTests.p:linux:y
