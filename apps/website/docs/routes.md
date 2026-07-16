# Routes

Below is a list of all routes on the website and a brief description of their purpose.

## account

- ### GET /account
  Private account dashboard showing recent posts, orders, etc.

- ### GET /account/devices
  List of user's registered devices.

- ### GET /account/devices/<device_id>
  Registered device details page.

- ### GET /account/favorites
  List of workshop posts saved by the current user.

  - ### GET /account/likes
  List of workshop posts liked by the current user.

- ### GET /account/orders
  List of user's orders.

- ### GET /account/orders/<order_id>
  Detailed order page with items, totals, shipping status, tracking, and receipt.

- ### GET /account/posts
  List of user's posts.

- ### GET /account/posts/<post_id>
  User's post page.

- ### GET /account/projects
  User's editor projects dashboard.

- ### GET /account/settings
  Main settings interface.

## admin

- ### GET /admin
  Administrative dashboard showing operational metrics and items requiring attention.

- ### GET /admin/analytics
  Analytics for commerce, Workshop activity, editor usage, devices, and support.

- ### GET /admin/audit-log
  Immutable history of administrative actions and configuration changes.

- ### GET /admin/comments
  Moderation interface for Workshop comments.

- ### GET /admin/compatibility
  Management interface for vehicle, sensor, adapter, and accessory compatibility records.

- ### GET /admin/content
  Management interface for documentation, FAQs, announcements, roadmap entries, and changelogs.

- ### GET /admin/devices
  Searchable table of registered and manufactured devices.

- ### GET /admin/devices/<device_id>
  Device ownership, hardware revision, firmware history, warranty, and diagnostic details.

- ### GET /admin/discounts
  Management interface for coupons, promotions, automatic discounts, and free-shipping rules.

- ### GET /admin/firmware
  Firmware release list with channels, rollout status, adoption, and failure metrics.

- ### GET /admin/firmware/<release_id>
  Firmware release details, files, compatibility, release notes, and rollout controls.

- ### GET /admin/inventory
  Inventory overview showing stock, reserved units, incoming units, damage, and adjustments.

- ### GET /admin/orders
  Searchable order-management table.

- ### GET /admin/orders/<order_id>
  Order details, payment history, fulfillment timeline, tracking, refunds, and internal notes.

- ### GET /admin/posts
  Management and moderation interface for Workshop posts.

- ### GET /admin/posts/<post_id>
  Detailed post moderation page with metadata, files, reports, revisions, and actions.

- ### GET /admin/products
  Product catalog management interface.

- ### GET /admin/products/<product_id>
  Product editing page for pricing, inventory, media, specifications, shipping, and SEO.

- ### GET /admin/reports
  Central queue for reported posts, comments, users, and files.

- ### GET /admin/returns
  Return request queue with approval, inspection, restocking, and refund controls.

- ### GET /admin/returns/<return_id>
  Return request details, inspection results, refund status, and administrative actions.

- ### GET /admin/settings
  Administrative settings for the store, Workshop, editor, firmware, email, and security.

- ### GET /admin/support
  Support ticket queue with assignment, status, priority, and category filters.

- ### GET /admin/support/<ticket_id>
  Support ticket conversation, attachments, linked orders, devices, and internal notes.

- ### GET /admin/users
  Searchable user table with account, commerce, moderation, and device information.

- ### GET /admin/users/<user_id>
  Detailed user page with account status, orders, posts, devices, support history, and notes.

- ### GET /admin/warranty
  Warranty claim queue with diagnosis, approval, replacement, repair, and rejection controls.

- ### GET /admin/warranty/<claim_id>
  Warranty claim details, diagnostics, evidence, repair, replacement, and resolution history.

- ### GET /admin/roles
  Management interface for administrative roles and permissions.

## auth

- ### GET /forgot-password
  Request a password reset email.

- ### POST /forgot-password
  Sends a password reset email when the account is eligible.

- ### GET /login
  Login form.

- ### POST /login
  Authenticates the user and creates a session.

- ### POST /logout
  Logout route.

- ### GET /register
  Account registration form.

- ### POST /register
  Creates a new account/

- ### POST /resend-verification
  Resend the verification message.

- ### GET /reset-password/<token>
  Password reset form opened from a valid reset link.

- ### POST /reset-password/<token>
  Validates the token and sets anew password.

- ### GET /verify-email/<token>
  Verifies a user's email address.

## cart

- ### GET /cart
  Current session or user cart.

- ### POST /cart/items
  Adds a product to the current cart.

- ### PATCH /cart/items/<item_id>
  Updates an item's quantity or configuration.

- ### DELETE /cart/items/<item_id>
  Removes an item from the cart.

## checkout

- ### GET /checkout/success
  Confirmation after successful payment

- ### GET /checkout/cancel
  Returned here if the user cancels checkout.

## company

- ### GET /about
  Company story and product purpose.

- ### GET /changelog
  Released hardware, firmware, editor, and website changes.

- ### GET /contact
  General company, press, business, and partnership inquiries.

- ### GET /roadmap
  Public roadmap.

## docs

- ### GET /docs
  Documentation home.

- ### GET /docs/<path>
  Documentation for a specific topic.

## editor

- ### GET /editor
  Gauge face editor.

## legal

- ### GET /legal
  Index of all legal policies and customer terms.

- ### GET /legal/terms
- ### GET /legal/privacy
- ### GET /legal/shipping
- ### GET /legal/returns
- ### GET /legal/warranty
- ### GET /legal/community-guidelines
- ### GET /legal/safety
- ### GET /legal/licenses
- ### GET /legal/cookies
- ### GET /legal/copyright

## products

- ### GET /products
  Product catalog.

- ### GET /products/<slug>
  Product detail page.

## support

- ### GET /downloads
  Firmware, manuals, utilities, diagrams, drivers, and sample files.

- ### GET /faq
  Common product, compatibility, editor, order, shipping, and warranty questions.

- ### GET /support
  Support landing page.

- ### GET /support/contact
  Technical support request.

- ### GET /support/return-request
  Start a return.

- ### GET /support/tickets
  List of the current user's support tickets.

- ### GET /support/tickets/<ticket_id>
  Support ticket conversation, status, attachments, and related information.

- ### GET /support/warranty-claim
  Submit a warranty claim.

## system routes

- ### GET /.well-known/security.txt
- ### GET /maintenance
- ### GET /robots.txt
- ### GET /sitemap.xml

## user

- ### GET /user/<username>
  Public profile page.

- ### GET /user/<username>/posts
  List of user's posts.

## workshop

- ### /workshop
  Workshop discovery page with search, sorting, filtering, and pagination.

- ### /workshop/<post_id>
  Public detail page for a published package.