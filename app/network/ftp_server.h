#ifndef NETWORK_FTP_SERVER_H
#define NETWORK_FTP_SERVER_H

#include <Arduino.h>
#include <FS.h>
#include "FTPServer.h"

/**
 * FTP Server class for file transfer over network
 */
class FtpServer {
public:
    FtpServer(FS& filesystem);
    ~FtpServer();

    /**
     * Initialize and start the FTP server
     * @return true if successful, false otherwise
     */
    bool begin();

    /**
     * Stop the FTP server
     */
    void stop();

    /**
     * Handle FTP client requests (call in loop or task)
     */
    void handle();

private:
    FTPServer* ftpServer;
    FS& fs;
};

#endif // NETWORK_FTP_SERVER_H