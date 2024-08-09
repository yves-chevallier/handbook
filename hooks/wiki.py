""" Provide wikipedia link to the page """
import logging
import requests
import re
from ruamel.yaml import YAML
log = logging.getLogger('mkdocs')

def get_wiki_link(keyword, lang):
    search_url = f"https://{lang}.wikipedia.org/w/api.php"

    params = {
        'action': 'query',
        'list': 'search',
        'format': 'json',
        'srlimit': 5,
        'srsearch': keyword,
    }

    headers = {
        'User-Agent': 'MyWikipediaApp/1.0 (https://mywebsite.com)'
    }

    response = requests.get(search_url, headers=headers, params=params)
    data = response.json()

    if 'query' in data and 'search' in data['query'] and len(data['query']['search']) > 0:
        page_title = data['query']['search'][0]['title']
        page_url = f"https://{lang}.wikipedia.org/wiki/{page_title.replace(' ', '_')}"
        return page_url, page_title
    else:
        return None

def get_wiki_summary(lang, page_title):
    summary_url = f"https://{lang}.wikipedia.org/api/rest_v1/page/summary/{page_title}"
    response = requests.get(summary_url)
    data = response.json()
    keep_keys = 'title', 'thumbnail', 'timestamp', 'description', 'extract', 'tid'
    return {k: v for k, v in data.items() if k in keep_keys}

links = {}

def on_config(config):
    global links
    yaml = YAML()
    yaml.preserve_quotes = True
    with open('links.yml') as f:
        links = yaml.load(f)
    if 'wikipedia' not in links:
        log.warning("No section wikipedia in links.yml")

def on_page_markdown(markdown, page, config, files):
    def replace_link(link):
        keyword = link.group(2)
        if not isinstance(links['wikipedia'], dict):
            links['wikipedia'] = {}
        if keyword not in links['wikipedia']:
            result = get_wiki_link(keyword, 'fr')
            if result is None:
                log.error(f"Unable to find wikipedia link for keyword: {keyword}")
                return link.group(0)
            url, title = result
            if url is not None:
                links['wikipedia'][keyword] = {
                    'url': url,
                    'title': title,
                    'validated': False,
                }
                log.warning(f"New wiki link discovered: {keyword}, guessing to be {title}")

        if 'title' not in links['wikipedia'][keyword] and 'url' in links['wikipedia'][keyword]:
            title = links['wikipedia'][keyword]['url'].split('/')[-1]
            links['wikipedia'][keyword]['title'] = title

        if 'tid' not in links['wikipedia'][keyword]:
            log.info(f"Updating wikipedia summary for keyword: {keyword}")
            summary = get_wiki_summary('fr', links['wikipedia'][keyword]['title'])
            if summary:
                links['wikipedia'][keyword].update(summary)
            else:
                log.error(f"Unable to find wikipedia summary for keyword: {keyword}")
                return link.group(0)

        return f"{link.group(1)}{links['wikipedia'][keyword]['url']}{link.group(3)}"

    with open('links.yml', 'w') as f:
        yaml = YAML()
        yaml.dump(links, f)

    return re.sub(r'(\[[^\]]+\]\()wiki:([^\)]+)(\))', replace_link, markdown)

def on_post_build(config):
    with open('links.yml', 'w') as f:
        yaml = YAML()
        yaml.dump(links, f)