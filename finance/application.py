import os

from datetime import datetime
from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session, jsonify
from flask_session import Session
from tempfile import mkdtemp
from werkzeug.exceptions import default_exceptions, HTTPException, InternalServerError
from werkzeug.security import check_password_hash, generate_password_hash

from helpers import apology, login_required, lookup, usd

# Configure application
app = Flask(__name__)

# Ensure templates are auto-reloaded
app.config["TEMPLATES_AUTO_RELOAD"] = True


# Ensure responses aren't cached
@app.after_request
def after_request(response):
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


# Custom filter
app.jinja_env.filters["usd"] = usd

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_FILE_DIR"] = mkdtemp()
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///finance.db")

# Make sure API key is set
if not os.environ.get("API_KEY"):
    raise RuntimeError("API_KEY not set")


@app.route("/", methods=["GET", "POST"])
@login_required
def index():
    """Show portfolio of stocks"""

    # Get ticker, company name, quantity
    portfolio = db.execute(
        "SELECT stock.ticker, stock.name, portfolio.quantity FROM stock JOIN users, portfolio ON users.id = portfolio.user_id AND stock.id = portfolio.stock_id WHERE users.id IN (SELECT id FROM users WHERE id = ?)", session["user_id"])

    # Get stock price
    stock_prices = {}
    total_price = float(session["cash"])
    for stock in portfolio:
        stock_prices[stock["ticker"]] = float((lookup(stock["ticker"]))["price"])
        total_price += (stock_prices[stock["ticker"]] * stock["quantity"])

    return render_template("index.html", portfolio=portfolio, stock_prices=stock_prices, cash=session["cash"], total=total_price)


@app.route("/buy", methods=["GET", "POST"])
@login_required
def buy():
    """Buy shares of stock"""

    # User performs buy action
    if request.method == "POST":

        # Store canonicalized symbol
        symbol = request.form.get("symbol").upper()

        # Ensure an order of positive int has been placed
        if not request.form.get("shares").isdigit() or int(request.form.get("shares")) < 0:
            return apology("invalid number of shares", 400)

        # Ensure stock ticker exists
        if not lookup(symbol):
            return apology("Symbol not found", 400)
        else:
            # Add stock into stock database
            stock_info = lookup(symbol)

            stock_catalog = db.execute("SELECT * FROM stock WHERE ticker = ?", request.form.get("symbol").upper())
            if len(stock_catalog) == 0:
                db.execute("INSERT INTO stock (ticker, name) VALUES (?, ?)", stock_info['symbol'], stock_info['name'])

            stock_id = db.execute("SELECT id FROM stock WHERE ticker = ?", stock_info["symbol"])
            stock_id = stock_id[0]["id"]

        # Ensure user has enough buying power to complete purchase
        stock_info = lookup(symbol)
        purchase_price = float(request.form.get("shares")) * stock_info["price"]
        if session["cash"] - purchase_price < 0:
            return apology("insufficient funds to complete purchase", 400)

        # Record purchase
        db.execute("INSERT INTO account_history (user_id, stock_id, transaction_type, quantity, price) VALUES (?, ?, 'BUY', ?, ?)",
                   session["user_id"], stock_id, request.form.get("shares"), stock_info["price"])

        # Update user's portfolio
        if len(db.execute("SELECT * FROM portfolio WHERE user_id = (?) AND stock_id = (?)", session["user_id"], stock_id)) != 0:
            db.execute("UPDATE portfolio SET quantity = quantity + (?) WHERE user_id = (?) AND stock_id = (?)",
                       request.form.get("shares"), session["user_id"], stock_id)
        else:
            db.execute("INSERT INTO portfolio (user_id, stock_id, quantity) VALUES (?, ?, ?)",
                       session["user_id"], stock_id, request.form.get("shares"))

        # Update user's balance
        session["cash"] = session["cash"] - purchase_price
        db.execute("UPDATE users SET cash = ? WHERE id = ?", session["cash"], session["user_id"])

        flash("Purchase order successful", "info")
        return redirect("/")

    # User arrives at buy page
    else:
        return render_template("buy.html")


@app.route("/history")
@login_required
def history():
    """Show history of transactions"""
    # Get ticker, quantity,
    history = db.execute(
        "SELECT stock.ticker, quantity, price, transaction_type, date FROM account_history JOIN stock ON stock.id = account_history.stock_id WHERE user_id = (?) ORDER BY date DESC", session["user_id"])

    return render_template("history.html", history=history)


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":

        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 403)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 403)

        # Query database for username
        rows = db.execute("SELECT * FROM users WHERE username = ?", request.form.get("username"))

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]["hash"], request.form.get("password")):
            return apology("invalid username and/or password", 403)

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]
        session["username"] = rows[0]["username"]

        # Log their account cash
        session["cash"] = rows[0]["cash"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/quote", methods=["GET", "POST"])
@login_required
def quote():
    """Get stock quote."""

    # User hits quote button
    if request.method == "POST":

        symbol = request.form.get("symbol").upper()
        if not lookup(symbol):
            return apology("stock not found", 400)

        quote = lookup(symbol)
        stock_catalog = db.execute("SELECT * FROM stock WHERE ticker = ?", request.form.get("symbol").upper())
        if len(stock_catalog) == 0:
            db.execute("INSERT INTO stock (ticker, name) VALUES (?, ?)", quote['symbol'], quote['name'])

        return render_template("quoted.html", quote=quote)
    # User reaches page through link
    else:
        return render_template("quote.html")


@app.route("/register", methods=["GET", "POST"])
def register():
    """Register user"""

    # Forget any user_id
    session.clear()

    # User has pressed the register button
    if request.method == "POST":

        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 400)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 400)

        # Ensure passwords match
        elif request.form.get("password") != request.form.get("confirmation"):
            return apology("passwords do not match", 400)

        # Check username is not in use
        users = db.execute("SELECT username FROM users WHERE username = ?", request.form.get("username"))
        if len(users) != 0:
            return apology("username alread in use")

        # Store user credentials in database
        db.execute("INSERT INTO users (username, hash) VALUES (?, ?)",
                   request.form.get("username"), generate_password_hash(request.form.get("password")))

        return redirect("/")

    # User reached by clicking link, button, or direct URL
    else:
        return render_template("register.html")


@app.route("/sell", methods=["GET", "POST"])
@login_required
def sell():
    """Sell shares of stock"""
    if request.method == "POST":
        # Ensure user owns stock
        ptfolio = db.execute("SELECT ticker FROM stock JOIN portfolio ON portfolio.stock_id = stock.id WHERE portfolio.user_id = (?) AND stock.ticker = (?)",
                             session['user_id'], request.form.get('symbol'))
        if not ptfolio:
            return apology("Stock not in portfolio", 400)

        # Ensure an order of positive int has been placed
        user_shares = db.execute("SELECT quantity FROM portfolio WHERE user_id = (?) AND stock_id IN (SELECT id FROM stock WHERE ticker = (?))",
                                 session["user_id"], request.form.get('symbol'))
        if not request.form.get("shares").isdigit() or int(request.form.get("shares")) < 0 or int(request.form.get('shares')) > user_shares[0]["quantity"]:
            return apology("invalid number of shares", 400)

        # Get price of the stock and gains
        price = lookup(request.form.get("symbol"))
        gains = price["price"] * float(request.form.get("shares"))

        # Update user's cash and portfolio
        session["cash"] += gains
        stock_id = db.execute("SELECT id FROM stock WHERE ticker = (?)", request.form.get("symbol"))
        db.execute("UPDATE users SET cash = (?) WHERE id = (?)", session["cash"], session["user_id"])
        db.execute("UPDATE portfolio SET quantity = quantity - (?) WHERE user_id = (?) AND stock_id = (?)",
                   request.form.get("shares"), session["user_id"], stock_id[0]["id"])
        db.execute("DELETE FROM portfolio WHERE quantity = 0 AND user_id = (?)", session["user_id"])

        # Update transaction log
        db.execute("INSERT INTO account_history (user_id, stock_id, transaction_type, quantity, price) VALUES (?, ?, 'SELL', ?, ?)",
                   session["user_id"], stock_id[0]["id"], request.form.get("shares"), price["price"])

        flash("Sell order successful", "info")
        return redirect("/")
    else:
        portfolio = db.execute(
            "SELECT stock.ticker FROM stock JOIN users, portfolio ON users.id = portfolio.user_id AND stock.id = portfolio.stock_id WHERE users.id IN (SELECT id FROM users WHERE id = ?)", session["user_id"])
        return render_template("sell.html", stocks=portfolio)


@app.route("/cash", methods=["GET", "POST"])
@login_required
def cash():
    session["cash"] += float(request.form.get("cash"))
    db.execute("UPDATE users SET cash = (?) WHERE id = (?)", session["cash"], session["user_id"])
    flash("Deposit successful", "info")
    return redirect("/")


@app.route("/account", methods=["GET", "POST"])
@login_required
def account():
    user = db.execute("SELECT username, hash FROM users WHERE id = (?)", session["user_id"])
    if request.method == 'POST':
        # Check if old password is correct
        if not check_password_hash(user[0]['hash'], request.form.get('password')):
            flash('Wrong pasword')
            return render_template('account.html', username=user[0]['username'])
        # Check is new passwords match
        if request.form.get('new_password') != request.form.get('new_password_reentry'):
            flash("Passwords do not match")
            return render_template("account.html", username=user[0]['username'])

        # Update password in database
        db.execute('UPDATE users SET hash = (?) WHERE id = (?)', generate_password_hash(
            request.form.get('new_password')), session["user_id"])
        flash("Password updated successfully")
        return render_template('account.html', username=user[0]['username'])

    return render_template('account.html', username=user[0]["username"])


def errorhandler(e):
    """Handle error"""
    if not isinstance(e, HTTPException):
        e = InternalServerError()
    return apology(e.name, e.code)


# Listen for errors
for code in default_exceptions:
    app.errorhandler(code)(errorhandler)