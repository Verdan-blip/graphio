from peewee import *
import datetime

DATABASE_NAME = 'app_scores.db'

db = SqliteDatabase(DATABASE_NAME)

class AppScore(Model):
    app_id = CharField(unique=True)
    score = FloatField(default=0.0)
    last_update = DateTimeField(default=datetime.datetime.now)

    class Meta:
        database = db

def init_db():
    db.connect()
    db.create_tables([AppScore])
