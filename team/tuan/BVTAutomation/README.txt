EXIV2 - Build Verification Test Automation
The application is to automate the build verification test. 
Version 2013-08-10: Update the Readme.txt

-------------------------------
REQUIRE: 
+ node.js must be installed. (http://nodejs.org/)
+ svn client

-------------------------------
Feature:
+ Update new revision.
+ Run the list of commands (BVT - test cases) for the revision if the revision contains KEYWORDS.
+ Email the results.
+ Create http server which allows user to view the result as well as run BVT manually. 

-------------------------------
SETUP:
*** Step 1: unzip node_modules/socket.io.zip
+ socket.io is a node.js module to allow http real-time update. I zipped this directory in order to upload it to svn server easily.
*** Step 2: settings
+ Open mailformat.txt to update the email address you want the result to be sent to.
+ Open requestHandlers.js:
    + Update the commands you want to run at line 29 (update the commands array).
    + Update the keywords which is used to indicate the revision need running BVT or not at line 43.
*** Step 3: checkout the branch you want to run BVT. (if you have already checked out, please skip this step)
*** Step 4: go to the branch directory "cd <the_branch_just_check_out>"
*** Step 5: run "node <bvt_dir>/index 8888 <the_branch_name>"
+ 8888 is the port for bvt, you can change it to whatever port you want.
+ You may want to set up this BVT automation for some branches, so <the_branch_name> is used to identify them.
+ If you get the error "listen EADDRINUSE" when running the above command, you need to change the port because it's occupied.
*** Step 6: 
+ At this step, you should have a http server running (please check http://localhost:8888), you can manually run BVT and view the result via that.
+ To make it automatic, you need to set up crontab (or schedule). 
crontab example: (check new build every half an hour).
0 */2 * * * curl -s http://localhost:8888/bvt

-------------------------------
Version History: None.

-------------------------------
More info: nhudinhtuan@gmail.com