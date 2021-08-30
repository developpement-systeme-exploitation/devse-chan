import asyncio
import time
import confuse
import re
from devsechan.irc import IRC
from devsechan.discord import Discord
from discord import Webhook, AsyncWebhookAdapter
import aiohttp

class DevSEChan:

    def __init__(self):
        self.config = confuse.Configuration('devsechan')
        self.loop = asyncio.get_event_loop()
        self.irc = IRC(self, self.config['irc'])
        self.discord = Discord(self, self.config['discord'])

    def get_member_ping_to_id(self, nickname, guild):
        member = guild.get_member_named(nickname)
        if member is not None:
            return f"<@{member.id}>"
        return None

    def convert_irc_mentions_to_discord(self, message, guild):
        mentions_regex = r"(?<=@)[a-zA-Z0-9]*"
        mentions_match = re.finditer(mentions_regex, message, re.MULTILINE)

        for mention in mentions_match:
            nickname = mention.group()
            member_id = self.get_member_ping_to_id(nickname, guild)
            if member_id is not None:
                message = message.replace('@' + nickname, member_id)

        return message

    async def to_discord(self, nick, message):
        webhook_url = self.config['discord']['webhook'].get()
        channel_id = self.config['discord']['channel'].get()
        channel = self.discord.get_channel(channel_id)
        guild = channel.guild
        converted_message = self.convert_irc_mentions_to_discord(message, guild)
        avatar = None
        member = guild.get_member_named(nick)
        if member is not None:
            avatar = member.avatar_url

        async with aiohttp.ClientSession() as session:
            webhook = Webhook.from_url(webhook_url, adapter=AsyncWebhookAdapter(session))
            await webhook.send(converted_message, username=nick, avatar_url=avatar)

    def to_irc(self, author, msg_list):
        target = self.config['irc']['channel'].get()
        for msg in msg_list:
            if len(msg) > 0:
                time.sleep(1)
                self.irc.send('PRIVMSG', target=target,
                    message=f"<\x036{author}\x0F> {msg}")
                time.sleep(2)

    def run(self):
        token = self.config['discord']['token'].get()
        self.loop.create_task(self.irc.connect())
        self.loop.create_task(self.discord.start(token))
        self.loop.run_forever()
