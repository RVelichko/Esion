#!/bin/bash
curl -v --output \
-H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8" \
-H "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7" \
-H "Cache-Control: max-age=0" \
-H "Connection: keep-alive" \
-H "Host: api.opencagedata.com" \
-H "Upgrade-Insecure-Requests: 1" \
-H "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36" \
-X GET https://api.opencagedata.com/geocode/v1/json \
-d "key=85e723a484ca41d88e3adafcb1218d34&q=МО Истра&pretty=1&no_annotations=1&language=ru"

#curl -v \
#-d "sco=longlat&lang=ru_RU&format=json&apikey=ad12ff63-587b-42c7-b1e1-e8b8b0913cda&geocode=%D0%9C%D0%9E%20%D0%B3.%D0%98%D1%81%D1%82%D1%80%D0%B0" \
#-H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8" \
#-H "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7" \
#-H "Cache-Control: max-age=0" \
#-H "Connection: keep-alive" \
#-H "Host: geocode-maps.yandex.ru" \
#-H "Upgrade-Insecure-Requests: 1" \
#-H "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36" \
#-X GET https://geocode-maps.yandex.ru/1.x/

#curl -v -s https://geocode-maps.yandex.ru/1.x/?sco=longlat&lang=ru_RU&format=json&apikey=ad12ff63-587b-42c7-b1e1-e8b8b0913cda&geocode=%D0%9C%D0%9E%20%D0%B3.%D0%98%D1%81%D1%82%D1%80%D0%B0


#-H "If-None-Match: W/"550-a8oOzdgx2wyb5FGi2AoafDkKfbQ"" \
#-H "Cookie: yandexuid=8834795441565252781; i=hgRw0GT+YzSpvbaEkEY4s9skOzXa5R4GqDNhizTpl3sCLuMCecUlvNUJ4T1pdGNr+W4oyEj666IiAE6qf8WviG1rvVk=; mda=0; _ym_uid=1565803467118665367; _ym_d=1565803467; yp=1880612781.yrtsi.1565252781#1581800245.szm.1:1920x1080:1920x851; my=YwA=; _csrf=bYAj_1FYFMRVF3-kE6mPrhEg; _fbp=fb.1.1566977472293.95055347; _ga=GA1.2.304715684.1568490047; ys=ymrefl.491D4C6F30A2EB21; _ym_wasSynced=%7B%22time%22%3A1569258227068%2C%22params%22%3A%7B%22eu%22%3A0%7D%2C%22bkParams%22%3A%7B%7D%7D; _ym_isad=1; _ym_visorc_146685=w; _ym_visorc_34230975=w; _ym_visorc_21930706=w; _ym_visorc_43169114=b" \
#-H "Accept-Encoding: gzip, deflate, br" \
