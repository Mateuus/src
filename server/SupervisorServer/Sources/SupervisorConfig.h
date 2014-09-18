#pragma once

class CSupervisorConfig
{
  public:
	std::string	masterIp_;
	int		masterPort_;

	DWORD		serverGroup_;
	std::string	serverName_;
	int		maxPlayers_;
	int		maxGames_;
	
	int		uploadLogs_;

	int		portStart_;
	std::string	externalIpStr_;
	DWORD		externalIpAddr_;
	void		ParseExternalIpAddr();

	std::string	gameServerExe_;
	
  public:
	CSupervisorConfig();
};

extern CSupervisorConfig* gSupervisorConfig;
