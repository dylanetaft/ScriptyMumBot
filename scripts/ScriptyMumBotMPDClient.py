import os
import grpc
import proto.MumBot_pb2
import re
import glob
import subprocess

youtube_dl_path = '../mpd/tmp/';
mpd_music_path = '../mpd/music/';
mpc_command = "mpc -p 7701 -h localhost";



channel = grpc.insecure_channel('localhost:50080')
stub = proto.MumBot_pb2.MumBotRPCStub(channel)
req   = proto.MumBot_pb2.TextMessageRequest(regex_search_pattern = "!(add|vol|skip).*");
response = stub.SubscribeToTextMessages(req)
for res in response:
    m = re.search('(^!\w*)(\s*)([";<>%\s\/\w\d:\.\?=\-&]*)',res.msg)
    if m:
        param = m.group(3)
        command = m.group(1)
        #command with param, whitelisting safe chars
        if command == '!add':
            m = re.search('(<a href=")(.*)(["\'])',param)
            if (m):
                url = m.group(2)
                cmdtoexecute = "youtube-dl -o " + "'"  + youtube_dl_path + "%(title)s.%(ext)s" + "'" + " --audio-quality 1 -x " + "'" + url + "'";
                os.system(cmdtoexecute)
                files = glob.glob(youtube_dl_path + '*')
                files.sort(key=os.path.getmtime)
                for file in files:
                    filename = file[len(youtube_dl_path):] #slice dir name from str
                    safefilename = re.sub('[^a-zA-Z0-9_\s\.]','_',filename)
                    os.rename(file,mpd_music_path + safefilename)
                    cmdtoexecute = mpc_command + " -w update"
                    os.system(cmdtoexecute)
                    cmdtoexecute = mpc_command + " -w add " + "'" + safefilename + "'"
                    os.system(cmdtoexecute)
                    p = os.popen(mpc_command + " current")
                    cmdoutput = p.readline()
                    if (cmdoutput == ""):
                        cmdtoexecute = mpc_command + " -w play"
                        os.system(cmdtoexecute)




