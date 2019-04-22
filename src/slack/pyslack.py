import os
import requests

TOKEN = 'butt'

def upload_file(
        filepath,
        channels,
        filename=None,
        content=None,
        title=None,
        initial_comment=None):
    """Upload file to channel

    Note:
        URLs can be constructed from:
        https://api.slack.com/methods/files.upload/test
    """

    if filename is None:
        filename = os.path.basename(filepath)

    data = {}
    data['token'] = TOKEN
    data['file'] = filepath
    data['filename'] = filename
    data['channels'] = channels

    if content is not None:
        data['content'] = content

    if title is not None:
        data['title'] = title

    if initial_comment is not None:
        data['initial_comment'] = initial_comment

    filepath = data['file']
    files = {
        'file': (filepath, open(filepath, 'rb'), 'image/jpg', {
            'Expires': '0'
        })
    }
    data['media'] = files
    response = requests.post(
        url='https://slack.com/api/files.upload',
        data=data,
        headers={'Accept': 'application/json'},
        files=files)

    return response.text


my_file = {
  'file' : ('hallc_logo.jpg', open('hallc_logo.jpg', 'rb'), 'jpg')
}

payload={
  "filename":"hallc_logo.jpg", 
  "token":TOKEN, 
  "channels":['#testing'], 
}

r = requests.post("https://slack.com/api/files.upload", params=payload, files=my_file)

