const jwt = require('jsonwebtoken');

module.exports = async function (context, req) {
  context.log('JavaScript HTTP trigger function processed a request.');

  const secret = 'Insert secret here';
  context.log(req);
  const id = req.query.id || (req.body && req.body.id) || undefined;

  console.log('ID:', id);

  if (!id) {
    context.res = {
      status: 500 /* Defaults to 200 */,
      body: 'You need to provide a token id!',
    };
    return;
  }

  var token = jwt.sign({ id: id }, secret);

  context.res = {
    // status: 200, /* Defaults to 200 */
    body: token,
  };
};
