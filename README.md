# fr24monitoring

fr24monitoring fetches informations from FlightRadar24 website and sends notifications when interesting aircrafts are planned for your airport


# Releases

For Windows, you can download latest release (v0.0.1) here:
 * https://github.com/nefethael/fr24monitoring/releases/download/v0.0.1/fr24monitoring-v0.0.1.zip


# Settings

Settings are made in `setup.ini` file which resides in same folder as application.

* Telegram

You can be notified by Telegram, you need to:

1. create a channel
2. create a bot (send a message to @BotFather and follow instruction) and save token
3. invite bot in channel
4. retrieve chat id [see here](https://www.alphr.com/find-chat-id-telegram/)

Then complete setup.ini with corresponding informations:

    ;telegram_token=123:ABC
    ;telegram_chat=-666

* Airport of interest

Set your airport, for example for Bordeaux (LFBD):

    airport=bod

# Filtering

fr24monitoring can send a notification when aircraft is newly planned for departure or landing at chosen airport or if there are information updates.
Some aircrafts are common and we don't want to be notified for them, so there is a filters.json file which defines that.
Filter algorithm is the following:


    IF fetched_aircraft.airline IN common_airline:
        IF fetched_aircraft.model IN common_aircraft OR fetched_aircraft.model IN common_shortcraft:
            DISCARD


