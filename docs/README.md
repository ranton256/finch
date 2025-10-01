# Finch Documentation Site

This directory contains the GitHub Pages documentation site for the Finch Graphics Library.

## Local Testing

To test the site locally, you can use any static file server. For example:

```bash
# Using Python 3
python3 -m http.server 8000

# Using Python 2
python -m SimpleHTTPServer 8000

# Using Node.js http-server
npx http-server
```

Then visit http://localhost:8000 in your browser.

## Generating Screenshots

The example screenshots are automatically generated from the example programs. To regenerate them:

```bash
# From the project root directory
make docs
```

This will:
1. Build the screenshot generator
2. Run it to create PNG images from the examples
3. Save the images to `docs/images/`

## File Structure

```
docs/
├── index.html          # Landing page
├── tutorial.html       # Complete tutorial
├── api.html            # API reference
├── examples.html       # Example programs showcase
├── style.css           # Shared stylesheet
├── images/             # Screenshots and visual tests
│   ├── example_*.png   # Example program screenshots
│   └── visual_test_*.png  # Integration test outputs
├── _config.yml         # GitHub Pages configuration
└── README.md           # This file
```

## Publishing to GitHub Pages

1. Push the `docs/` directory to GitHub
2. Go to repository Settings → Pages
3. Set source to "Deploy from a branch"
4. Select branch: `main` (or your default branch)
5. Select folder: `/docs`
6. Click Save

GitHub will automatically build and publish the site at:
`https://<username>.github.io/<repository>/`

## Updating Documentation

### Content Updates

- Edit the HTML files directly
- Follow the existing structure and styling
- Test locally before committing

### Screenshot Updates

- Modify the `screenshot_generator.c` if needed
- Run `make docs` to regenerate images
- Commit the updated images

### API Changes

- Update `api.html` when adding/changing functions
- Keep it synchronized with the actual code in `finch.h`

### Tutorial Updates

- Edit `tutorial.html` to add new sections or examples
- Ensure code examples are tested and work correctly

## Maintenance

- Screenshots are version-controlled in git
- Images are optimized PNGs (typically < 15KB each)
- All HTML validates as HTML5
- Responsive design works on mobile devices
