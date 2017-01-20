# -*- coding: utf-8 -*-
"""
Created on Fri Jul  8 11:48:27 2016

@author: zamihos
"""

import time
import tweepy
import datetime, csv
import configparser
import json
import codecs

#reload(sys)  
#sys.setdefaultencoding('UTF-8')

def removeEntersAndCommas(tweet):
    tweet = tweet.replace(',','')
    tweet = tweet.replace('\n','')
    tweet = tweet.replace('\r','')
    return tweet    
    
print('Hello...')
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

auth = tweepy.OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
auth.set_access_token(OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
api = tweepy.API(auth)

cities = config['TwitterParser1']['cities'].split(',')
cities_WOEID = config['TwitterParser1']['cities_WOEID'].split(',')
continent = config['TwitterParser1']['continent'].split(',')


trends_list = []
for i in range(0,len(cities)):
    try:
        c = api.trends_place(cities_WOEID[i]) 
        city = c[0]['locations'][0]['name']
        trends = c[0]['trends']
        for t in trends:
            #print(t['name'].encode("utf-8"))
            trends_list.append(city+","+removeEntersAndCommas(t['name'])+","+str(t['tweet_volume'])+','+continent[i]+"\n")
    except tweepy.TweepError:
        time.sleep(60*15+15)
        continue
    
filePath = '.\\SampleData\\'
with codecs.open(filePath+'trending_topics.csv','w',"utf-8") as csvfile:
    csvfile.write('city,topic,volume,continent\n')
    for t in trends_list:
        csvfile.write(t)
csvfile.close()  
    