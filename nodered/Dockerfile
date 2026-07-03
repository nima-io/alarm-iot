FROM nodered/node-red:4.1

# Install dashboard module at image build time — no runtime permission issues
RUN npm install --prefix /usr/src/node-red @flowfuse/node-red-dashboard
