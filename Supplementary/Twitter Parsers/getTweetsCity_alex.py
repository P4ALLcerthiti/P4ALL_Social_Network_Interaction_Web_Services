import time
import tweepy
import datetime, csv
import configparser

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

keywordsList = []
keywordsList = config['TwitterParser2']['keywordsList'].split(',')

cities = []
cities = config['TwitterParser2']['cities'].split(',')

cities_latlng = []
cities_latlng = config['TwitterParser2']['cities_latlng'].split(';')

#geo = config['TwitterParser1']['geocode_lat']+','+config['TwitterParser1']['geocode_lon']+','+config['TwitterParser1']['geocode_radius']
language = config['TwitterParser2']['lang']

#since_date = config['TwitterParser1']['since_date']
#until_date = config['TwitterParser1']['until_date']

auth = tweepy.OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
auth.set_access_token(OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
api = tweepy.API(auth)

# Get twitter ids and with a condition to avoid limitation errors
recordsPerCity = float(config['TwitterParser2']['recordsPerCity'])
recordsPerCity = int(recordsPerCity)
#keyword = ['paris']

userIdsList = []
tweetIdsList = []
tweetCreated_atList = []
screen_nameList = []
tweetTypeList = []
authorList = []
textList = []
cityList = []

filePath = '.\\SampleData\\'

print('Start...\n')


csvfile_list = []
for city in range(0,len(cities)):
    for keyword in keywordsList:
        counter = 0
        userIdsList = []
        tweetIdsList = []
        tweetCreated_atList = []
        screen_nameList = []
        tweetTypeList = []
        authorList = []
        textList = []
        cityList = []
        
        geo = cities_latlng[city]
        print("Test...\n")        
        print(keyword)
        print(cities[city])
        print(geo)
        c = tweepy.Cursor(api.search, q=keyword, geocode=geo,lang='en').items()    
        
        while True:
            try:
                tweet = c.next()
                text = removeEntersAndCommas(tweet.text) 
                #print(text)
                if(keyword.lower() in text.lower()):
                    if hasattr(tweet, 'retweeted_status'):
                      tweetType = ('r')
                      author = tweet.retweeted_status.id
                    else: 
                        tweetType = 'o'
                        author = tweet.id
                    userId = tweet.user.id
                    tweetId = tweet.id
                    tweetCreated_at = tweet.created_at
                    screen_name = tweet.user.screen_name
                    counter = counter+1
                    csvfile_list.append(str(tweetId)+','+str(author)+','+keyword+','+str(tweetCreated_at)+','+str(userId)+','+screen_name+','+tweetType+','+cities[city]+','+str(text.encode("utf-8"))+'\n')
                    
                    if counter >= recordsPerCity:
                        break;
            except tweepy.TweepError:
                time.sleep(60*15+15)
                continue
            except StopIteration:
                break
    
with open(filePath+'twitterIds_dataset_cities_new_Alex.csv','wt') as csvfile:
    csvfile.write('tweetId,author,keyword,tweetCreated_at,userId,screen_name,tweetType,city,text\n')
    for line in csvfile_list:
        csvfile.write(line)
csvfile.close()    