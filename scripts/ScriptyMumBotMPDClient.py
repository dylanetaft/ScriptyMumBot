import os
import grpc
import proto.MumBot_pb2
import re
import glob
import subprocess

youtube_dl_path = '../mpd/tmp/';
mpd_music_path = '../mpd/music/';
playlist_path = './playlists/'
mpc_command = "mpc -p 7701 -h localhost";

channel = grpc.insecure_channel('localhost:50080')
stub = proto.MumBot_pb2.MumBotRPCStub(channel)

lastFileList_ = []


def say(msg):
    pmsg = proto.MumBot_pb2.TextMessage(msg=msg)
    response = stub.Say(pmsg)



req   = proto.MumBot_pb2.TextMessageRequest(regex_search_pattern = "!(add|vol|skip).*");
response = stub.SubscribeToTextMessages(req)
for res in response:
    m = re.search('(^!\w*)(\s*)([";<>%\s\/\w\d:\.\?=\-&]*)',res.msg)
    if m:
        print("matches");
        param = m.group(3)
        command = m.group(1)
        #command with param, whitelisting safe chars
        if command == '!add':
            m = re.search('(<a href=")(.*)(["\'])',param)
            if (m):
                url = m.group(2)
                cmdtoexecute = "youtube-dl --no-mtime -o " + "'"  + youtube_dl_path + "%(title)s.%(ext)s" + "'" + " --audio-quality 1 -x " + "'" + url + "'";
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
            else:
                m = re.search('^\d*$',param) #queue a local song
                if m:
                    index = int(param)
                    if len(lastFileList_) > 0 and index <= len(lastFileList_) - 1:
                        filename = lastFileList_[index]
                        cmdtoexecute = mpc_command + " -w add " + "'" + filename + "'"
                        os.system(cmdtoexecute)
                        p = os.popen(mpc_command + " current")
                        cmdoutput = p.readline()
                        if (cmdoutput == ""):
                            cmdtoexecute = mpc_command + " -w play"
                            os.system(cmdtoexecute)
                        
                
        elif command == '!vol':
            m = re.search('^\d*$',param)
            if m:
                vol = command
                if (vol >= 0 and vol <= 100):
                    cmdtoexecute = mpc_command + " -w volume " + vol;
                    os.system(cmdtoexecute)
        elif command == '!skip':
            cmdtoexecute = mpc_command + " -w next"
            os.system(cmdtoexecute)

        elif command == '!help':
            say("!list - list downloaded music files")
            say("!list pattern - ex !list chrono - list files matching \"chrono\" ")
            say("!add # - ex !add 10 - add song 10 in the last displayed list of local files FUCKING USE THIS")
            say("!add http://youtubeurl - add youtube file to playlist")
            say("!vol # - set the volume, 0-100")
            say("!skip - this song sucks")

        elif command == '!list':
            
            files = glob.glob(mpd_music_path + '*')
            files.sort(key=os.path.getmtime)
            lastFileList_ = []
            count = 0
            for file in files:
                filename = file[len(mpd_music_path):] #slice dir name from str
                if param == "": #list all files
                    lastFileList_.append(filename)
                    say(msg=str(count) + " " + filename)
                    count = count + 1
                else:
                    foundstr = filename.lower().find(param.lower())
                    if foundstr != -1:
                        lastFileList_.append(filename)
                        say(msg=str(count) + " " + filename)
                        count = count + 1

            else:
                m = re.search('^\d*$',param)




