@echo off

set IDENTITY_FILE=%1
set USER=%2
set SRV=%3

ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo systemctl stop lottery-back"
ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo systemctl stop lottery-bot"

rem scp -i %IDENTITY_FILE% back/build/lottery-back %USER%@%SRV%:~/lottery/
rem scp -i %IDENTITY_FILE% back/_misc/libssl.so %USER%@%SRV%:~/lottery/
scp -i %IDENTITY_FILE% back/_misc/lottery-back.service %USER%@%SRV%:~/lottery/systemd/
scp -i %IDENTITY_FILE% back/_misc/ton-lite-client-test1.config.json %USER%@%SRV%:~/lottery/
rem ssh -i %IDENTITY_FILE% %USER%@%SRV% "chmod +x ~/lottery/lottery-back"

scp -i %IDENTITY_FILE% bot/bot %USER%@%SRV%:~/lottery/
scp -i %IDENTITY_FILE% bot/_misc/bot_env %USER%@%SRV%:~/lottery/
scp -i %IDENTITY_FILE% bot/_misc/lottery-bot.service %USER%@%SRV%:~/lottery/systemd/
ssh -i %IDENTITY_FILE% %USER%@%SRV% "chmod +x ~/lottery/bot"
rem ssh -i %IDENTITY_FILE% %USER%@%SRV% "chown %USER%:%USER% ~/lottery/bot_env"
rem ssh -i %IDENTITY_FILE% %USER%@%SRV% "chmod 400 ~/lottery/bot_env"

ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo cp ~/lottery/systemd/* /etc/systemd/system/"

ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo systemctl daemon-reload"

ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo systemctl start lottery-back"
ssh -i %IDENTITY_FILE% %USER%@%SRV% "sudo systemctl start lottery-bot"
