#pragma once

class CMasterUdpMonitor
{
  private:
	SOCKET		s_;
	
  public:
	CMasterUdpMonitor();
	~CMasterUdpMonitor();
	
	void		Start(int port);
	void		Stop();
	void		Tick();
};

extern CMasterUdpMonitor	gUdpMonitor;