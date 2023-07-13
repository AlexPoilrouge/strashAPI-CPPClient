FROM node:latest

WORKDIR /app

COPY package*.json ./

RUN npm install

COPY ./* ./

CMD ["sh", "-c", "mkdir -p /app/locker && rm -rf /app/locker/* && node test-api.js"]
