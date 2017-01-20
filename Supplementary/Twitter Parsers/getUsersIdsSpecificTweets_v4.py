# -*- coding: utf-8 -*-
"""
Created on Wed Jul  6 14:25:18 2016

@author: zamihos
"""

import time
import tweepy
from twython import Twython
import datetime, csv, string, pandas as pd 
import configparser

now = datetime.datetime.now()
day=int(now.day)
month=int(now.month)
year=int(now.year)

config = configparser.ConfigParser()
config.read('configuration.ini')

CONSUMER_KEY = config['AuthenticationParams']['consumer_key']
CONSUMER_SECRET = config['AuthenticationParams']['consumer_secret']
OAUTH_TOKEN = config['AuthenticationParams']['oauth_token']
OAUTH_TOKEN_SECRET = config['AuthenticationParams']['oauth_token_secret']

keyword = []
keyword = config['TwitterParser3']['keyword'].split(',')
users_limit = int(config['TwitterParser3']['users_limit'])

auth = tweepy.OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
auth.set_access_token(OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
api = tweepy.API(auth)

# Get twitter ids and with a condition to avoid limitation errors
userIds = []
tweetIds = []
tweetCreated_at = []
screen_name = []
tweets_counter=0
c = tweepy.Cursor(api.search,q=keyword,rpp=1,result_type="recent",include_entities=True,lang="en").items()   
while True:
    try:
        tweet = c.next()    
        tweets_counter=tweets_counter+1        
        print(tweets_counter)
        userIds.append(tweet.user.id)
        tweetIds.append(tweet.id)
        tweetCreated_at.append(tweet.created_at)
        screen_name.append(tweet.user.screen_name)
        if len(userIds)==users_limit:
            break
    except tweepy.TweepError:
        time.sleep(60*15)
        continue
    except StopIteration:
        break    

# Import ids to csv file
print("Import ids to csv file")
filePath = '.\\SampleData\\'
with open(filePath+'twitterIds_dataset.csv','wt') as nf:
    w=csv.writer(nf,delimiter=',',lineterminator='\n')    
    for i in range(0,len(userIds)):    
        row=[userIds[i],screen_name[i],tweetIds[i],tweetCreated_at[i]]        
        w.writerow(row)
    nf.close()

# Import data file with userId,screen name, tweetId, and time creation
inputFilePath = filePath+'twitterIds_dataset.csv'
f=open(inputFilePath)
data=csv.reader(f,delimiter=',')
data=[row for row in data]
f.close()
# Create a data frame
df = pd.DataFrame(data)
# Label column names for labes of first row of data frame
df = pd.DataFrame(data, columns = ['userId','screen_name','tweetId','created_at'])

counter = 0

#Get the followers of every user

with open(filePath+'followers.csv','wt') as nf:
    nf.write('userId,followers\n')
    for k in range(0,len(df['screen_name'])):
        w=csv.writer(nf,delimiter=',',lineterminator='\n') 
        counter = k    
        try:
            # Get followers ids
            followers_ids = []
            for page in tweepy.Cursor(api.followers_ids, screen_name = df['screen_name'][k]).pages():
                followers_ids.extend(page)
            for i in range(0,len(followers_ids)):    
                row=[df['userId'][k],followers_ids[i]]        
                w.writerow(row)
    
            if counter==len(df['screen_name']):
                break
        except tweepy.TweepError:
            time.sleep(60*15)
            print (k)
            continue
        except StopIteration:
            break
nf.close()   
     
#Get followings of every user
with open(filePath+'followings.csv','wt') as nf:
    nf.write('userId,followings\n')
    for k in range(0,len(df['screen_name'])):
        w=csv.writer(nf,delimiter=',',lineterminator='\n') 
        counter = k    
        try:
            # Get friends ids
            friends_ids = []
            for page in tweepy.Cursor(api.friends_ids, screen_name = df['screen_name'][k]).pages():
                friends_ids.extend(page)
                
            for i in range(0,len(friends_ids)):    
                row=[df['userId'][k],friends_ids[i]]        
                w.writerow(row)
    
            if counter==len(df['screen_name']):
                break
        except tweepy.TweepError:
            time.sleep(60*15)
            print (k)
            continue
        except StopIteration:
            break
nf.close()

#Get retweeters of every user
#with open('retweeted.csv','wt') as nf:
#    for k in range(0,len(df['screen_name'])):
#        w=csv.writer(nf,delimiter=',',lineterminator='\n') 
#        counter = k    
#        try:
#            # Get retweeters ids
#            public_tweets = api.search(df['screen_name'][k])
#            for tweet in public_tweets:
#                page = api.retweets(df['tweetId'][0])
#                if len(page) > 0:       
#                    retweetsId = []
#                    retweetsUserIds = []
#                    retweetsCreated_at = []        
#                    for retweet in page:
#                        retweetsId.append(retweet.id)
#                        retweetsUserIds.append(retweet.user.id)
#                        retweetsCreated_at.append(retweet.created_at)
#            
#                    w=csv.writer(nf,delimiter=',',lineterminator='\n')    
#                    for i in range(0,len(retweetsId)):    
#                        row=[df['userId'][k],retweetsId[i],retweetsUserIds[i],retweetsCreated_at[i]]        
#                        w.writerow(row)
#    
#            if counter==len(df['screen_name']):
#                break
#        except tweepy.TweepError:
#            time.sleep(60*15)
#            print (k)
#            continue
#        except StopIteration:
#            break
#nf.close()