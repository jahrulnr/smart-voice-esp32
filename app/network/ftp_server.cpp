#include "ftp_server.h"
#include "infrastructure/logger.h"
#include "config.h"

FtpServer::FtpServer(FS& filesystem) : ftpServer(nullptr), fs(filesystem) {}

FtpServer::~FtpServer() {
    if (ftpServer) {
        delete ftpServer;
    }
}

bool FtpServer::begin() {
    if (ftpServer) {
        Logger::warn("FTP", "FTP server already started");
        return true;
    }

    Logger::info("FTP", "Starting FTP server...");

    ftpServer = new FTPServer(fs);
    if (!ftpServer) {
        Logger::error("FTP", "Failed to create FTPServer instance");
        return false;
    }

    ftpServer->begin(FTP_USERNAME, FTP_PASSWORD);
    Logger::info("FTP", "FTP server started with user: %s", FTP_USERNAME);
    return true;
}

void FtpServer::stop() {
    if (ftpServer) {
        Logger::info("FTP", "Stopping FTP server...");
        ftpServer->stop();
        delete ftpServer;
        ftpServer = nullptr;
    }
}

void FtpServer::handle() {
    if (ftpServer) {
        ftpServer->handleFTP();
    }
}